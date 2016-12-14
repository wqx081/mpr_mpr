#include "mprmpr/server/server_base.h"

#include <sstream>
#include <string>
#include <vector>

#include <gflags/gflags.h>

#include "mprmpr/common/version_info.h"
#include "mprmpr/common/wire_protocol.pb.h"
#include "mprmpr/base/strings/strcat.h"
#include "mprmpr/base/strings/substitute.h"
#include "mprmpr/base/walltime.h"

#include "mprmpr/rpc/messenger.h"

#include "mprmpr/server/default_path_handlers.h"
#include "mprmpr/server/generic_service.h"
#include "mprmpr/server/glog_metrics.h"
#include "mprmpr/server/hybrid_clock.h"
#include "mprmpr/server/logical_clock.h"
#include "mprmpr/server/rpc_server.h"
#include "mprmpr/server/web_server.h"
#include "mprmpr/server/server_base_options.h"
#include "mprmpr/server/server_base.pb.h"

#include "mprmpr/util/atomic.h"
#include "mprmpr/util/jsonwriter.h"
#include "mprmpr/util/mem_tracker.h"
#include "mprmpr/util/metrics.h"
#include "mprmpr/util/monotime.h"
#include "mprmpr/util/net/sockaddr.h"
#include "mprmpr/util/spinlock_profiling.h"
#include "mprmpr/util/thread.h"
#include "mprmpr/util/env.h"
#include "mprmpr/util/rolling_log.h"

#include "mprmpr/util/oid_generator.h"

DEFINE_int32(num_reactor_threads, 4, 
"Number of libev reactor threads to start.");

DEFINE_int32(min_negotiation_threads, 0, 
"Minimum number of connection negotiation threads.");

DEFINE_int32(max_negotiation_threads, 50, 
"Maximum number of connection negotiation threads.");

DECLARE_bool(use_hybrid_clock);

using std::ostringstream;
using std::shared_ptr;
using std::string;
using std::vector;
using strings::Substitute;

namespace mprmpr {
namespace server {

namespace {

AtomicInt<int32_t> mem_tracker_id_counter(-1);

shared_ptr<MemTracker> CreateMemTrackerForServer() {
  int32_t id = mem_tracker_id_counter.Increment();
  string id_str = "server";
  if (id != 0) {
    StrAppend(&id_str, " ", id);
  }
  return shared_ptr<MemTracker>(MemTracker::CreateTracker(-1, id_str));
}

} // namespace


ServerBase::ServerBase(string name,
		       const ServerBaseOptions& options,
		       const string& metric_namespace)
    : name_(std::move(name)),
      mem_tracker_(CreateMemTrackerForServer()),
      metric_registry_(new MetricRegistry()),
      metric_entity_(METRIC_ENTITY_server.Instantiate(metric_registry_.get(),
			                              metric_namespace)),
      rpc_server_(new RpcServer(options.rpc_opts)),
      web_server_(new WebServer(options.webserver_opts)),
      result_tracker_(new rpc::ResultTracker(shared_ptr<MemTracker>(
                      MemTracker::CreateTracker(-1, "result-tracker",
                      mem_tracker_)))),
      is_first_run_(false),
      options_(options),
      stop_metrics_logging_latch_(1) {
  if (FLAGS_use_hybrid_clock) {
    clock_ = new HybridClock();
  } else {
    clock_ = LogicalClock::CreateStartingAt(Timestamp::kInitialTimestamp);
  }     

  CHECK_OK(StartThreadInstrumentation(metric_entity_, web_server_.get()));
  //TODO(wqx):
}

ServerBase::~ServerBase() {
  Shutdown();
}

Sockaddr ServerBase::first_rpc_address() const {
  vector<Sockaddr> addrs;
  WARN_NOT_OK(rpc_server_->GetBoundAddresses(&addrs),
    "Couldn't get bound RPC address");
  CHECK(!addrs.empty()) << "Not bound";
  return addrs[0];
}

Sockaddr ServerBase::first_http_address() const {
  vector<Sockaddr> addrs;
  WARN_NOT_OK(web_server_->GetBoundAddresses(&addrs),
    "Couldn't get bound webserver addresses");
  CHECK(!addrs.empty()) << "Not bound";
  return addrs[0];
}

const NodeInstancePB& ServerBase::instance_pb() const {
  return *DCHECK_NOTNULL(instance_pb_.get());
}

void ServerBase::GenerateInstanceID() {
  instance_pb_.reset(new NodeInstancePB);
  //TODO(wqx):
  ObjectIdGenerator oid_generator;
  std::string uuid = oid_generator.Next();
  instance_pb_->set_permanent_uuid(uuid);

  instance_pb_->set_instance_seqno(Env::Default()->NowMicros());
}

Status ServerBase::Init() {
  glog_metrics_.reset(new ScopedGLogMetrics(metric_entity_));
  //TODO(wqx):
  //tcmalloc
  RegisterSpinLockContentionMetrics(metric_entity_);

  InitSpinLockContentionProfiling();

  RETURN_NOT_OK_PREPEND(clock_->Init(), "Cannot initialize clock");

  //TODO(wqx):
  //RETURN_NOT_OK(security::InitKerberosForServer());
  //
  
  // Create the messenger
  rpc::MessengerBuilder builder(name_);

  builder.set_num_reactors(FLAGS_num_reactor_threads);
  builder.set_min_negotiation_threads(FLAGS_min_negotiation_threads);
  builder.set_max_negotiation_threads(FLAGS_max_negotiation_threads);
  builder.set_metric_entity(metric_entity());
  RETURN_NOT_OK(builder.Build(&messenger_));

  RETURN_NOT_OK(rpc_server_->Init(messenger_));
  RETURN_NOT_OK(rpc_server_->Bind());
  clock_->RegisterMetrics(metric_entity_);

  RETURN_NOT_OK_PREPEND(StartMetricsLogging(), 
		  "Could not enable metrics logging");

  result_tracker_->StartGCThread();

  return Status::OK();

}


void ServerBase::GetStatusPB(ServerStatusPB* status) const {
  status->mutable_node_instance()->CopyFrom(*instance_pb_);

  // RPC ports
  {
    vector<Sockaddr> addrs;
    CHECK_OK(rpc_server_->GetBoundAddresses(&addrs));
    for (const Sockaddr& addr : addrs) {
      HostPortPB* pb = status->add_bound_rpc_addresses();
      pb->set_host(addr.host());
      pb->set_port(addr.port());
    }
  }

  // HTTP ports
  {
    vector<Sockaddr> addrs;
    CHECK_OK(web_server_->GetBoundAddresses(&addrs));
    for (const Sockaddr& addr : addrs) {
      HostPortPB* pb = status->add_bound_http_addresses();
      pb->set_host(addr.host());
      pb->set_port(addr.port());
    }
  }

  VersionInfo::GetVersionInfoPB(status->mutable_version_info());
}

Status ServerBase::DumpServerInfo(const string& path,
		                  const string& format) const {
  ServerStatusPB status;
  GetStatusPB(&status);

  //TODO(wqx):
  if (format == "json") {
    string json = JsonWriter::ToJson(status, JsonWriter::PRETTY);
    RETURN_NOT_OK(WriteStringToFile(options_.env, Slice(json), path));
  } else {
    return Status::InvalidArgument("bad format", format);
  }

  LOG(INFO) << "Dumped server information to " << path;
  return Status::OK();
}

Status ServerBase::RegisterService(gscoped_ptr<rpc::ServiceIf> rpc_impl) {
  return rpc_server_->RegisterService(std::move(rpc_impl));
}

Status ServerBase::StartMetricsLogging() {
  if (options_.metrics_log_interval_ms <= 0) {
    return Status::OK();
  }

  return Thread::Create("server", 
		        "metrics-logger", 
			&ServerBase::MetricsLoggingThread,
			this, 
			&metrics_logging_thread_);
}


void ServerBase::MetricsLoggingThread() {
  RollingLog log(Env::Default(), FLAGS_log_dir, "metrics");

  const MonoDelta kWaitBetweenFailures = MonoDelta::FromSeconds(60);


  MonoTime next_log = MonoTime::Now();
  while (!stop_metrics_logging_latch_.WaitUntil(next_log)) {
    next_log = MonoTime::Now() +
    MonoDelta::FromMilliseconds(options_.metrics_log_interval_ms);

    std::ostringstream buf;
    buf << "metrics " << GetCurrentTimeMicros() << " ";

    vector<string> metrics;
    metrics.push_back("*");
    MetricJsonOptions opts;
    opts.include_raw_histograms = true;
    JsonWriter writer(&buf, JsonWriter::COMPACT);
    Status s = metric_registry_->WriteAsJson(&writer, metrics, opts);
    if (!s.ok()) {
      WARN_NOT_OK(s, "Unable to collect metrics to log");
      next_log += kWaitBetweenFailures;
      continue;
    }
    buf << "\n";
    s = log.Append(buf.str());
    if (!s.ok()) {
      WARN_NOT_OK(s, "Unable to write metrics to log");
      next_log += kWaitBetweenFailures;
      continue;
    }
  }

  WARN_NOT_OK(log.Close(), "Unable to close metric log");
}

std::string ServerBase::FooterHtml() const {
  return Substitute("<pre>$0\nserver uuid $1</pre>",
                    VersionInfo::GetShortVersionString(),
		    instance_pb_->permanent_uuid());
}

Status ServerBase::Start() {

  GenerateInstanceID();

  RETURN_NOT_OK(RegisterService(make_gscoped_ptr<rpc::ServiceIf>(
    new GenericServiceImpl(this))));

  RETURN_NOT_OK(rpc_server_->Start());

  AddDefaultPathHandlers(web_server_.get());
  //AddRpczPathHandlers(messenger_, web_server_.get());
  RegisterMetricsJsonHandler(web_server_.get(), metric_registry_.get());
  //TracingPathHandlers::RegisterHandlers(web_server_.get());
  web_server_->set_footer_html(FooterHtml());
  RETURN_NOT_OK(web_server_->Start());

  if (!options_.dump_info_path.empty()) {
    RETURN_NOT_OK_PREPEND(DumpServerInfo(options_.dump_info_path, options_.dump_info_format),
							                          "Failed to dump server info to " + options_.dump_info_path);
  }

  return Status::OK();
}

void ServerBase::Shutdown() {
  if (metrics_logging_thread_) {
    stop_metrics_logging_latch_.CountDown();
    metrics_logging_thread_->Join();
  }
  web_server_->Stop();
  rpc_server_->Shutdown();
}


} // namespace server
} // namespace mprmpr

#include "mprmpr/worker_server/heartbeater.h"

#include <gflags/gflags.h>
#include <glog/logging.h>
#include <memory>
#include <string>
#include <vector>

#include "mprmpr/common/wire_protocol.h"
#include "mprmpr/base/ref_counted.h"
#include "mprmpr/base/strings/substitute.h"

#include "mprmpr/master/master.h"
#include "mprmpr/master/master_rpc.h"
#include "mprmpr/master/master.proxy.pb.h"
#include "mprmpr/server/web_server.h"

#include "mprmpr/worker_server/worker_server.h"
#include "mprmpr/worker_server/worker_server_options.h"

//#include "mprmpr/tserver/ts_tablet_manager.h"

#include "mprmpr/util/thread.h"
#include "mprmpr/util/monotime.h"
#include "mprmpr/util/net/net_util.h"
#include "mprmpr/util/status.h"
#include "mprmpr/common/version_info.h"

DEFINE_int32(heartbeat_rpc_timeout_ms, 15000,
"Timeout used for the Worker->Master heartbeat RPCs.");

DEFINE_int32(heartbeat_interval_ms, 1000,
"Interval at which the Worker heartbeats to the master.");

DEFINE_int32(heartbeat_max_failures_before_backoff, 3,
"Maximum number of consecutive heartbeat failures until the "
"Worker Server backs off to the normal heartbeat interval, "
"rather than retrying.");

using google::protobuf::RepeatedPtrField;
using mprmpr::HostPortPB;
//using mprmpr::consensus::RaftPeerPB;
//using mprmpr::master::GetLeaderMasterRpc;
//using mprmpr::master::ListMastersResponsePB;
using mprmpr::master::Master;
using mprmpr::master::MasterServiceProxy;
using mprmpr::master::WorkerReportPB;
using mprmpr::rpc::RpcController;
using std::shared_ptr;
using strings::Substitute;

namespace mprmpr {
namespace worker_server {

namespace {

Status MasterServiceProxyForHostPort(const HostPort& hostport,
                                     const shared_ptr<rpc::Messenger>& messenger,
                                     gscoped_ptr<MasterServiceProxy>* proxy) {
  vector<Sockaddr> addrs;
  RETURN_NOT_OK(hostport.ResolveAddresses(&addrs));
  if (addrs.size() > 1) {
    LOG(WARNING) << "Master address '" << hostport.ToString() << "' "
                 << "resolves to " << addrs.size() << " different addresses. Using "
                 << addrs[0].ToString();
  }
  proxy->reset(new MasterServiceProxy(messenger, addrs[0]));
  return Status::OK();
}

} // namespace


class Heartbeater::Thread {
 public:
  Thread(const HostPort& master_address, WorkerServer* server);
  ~Thread() {}

  Status Start();
  Status Stop();
  void TriggerASAP();
  void GenerateIncrementWorkerReport(WorkerReportPB* report);
  void GenerateFullWorkerReport(WorkerReportPB* report);

  void MarkWorkerReportAcknowledged(const WorkerReportPB& report);

 private:
  void RunThread();
  Status ConnectToMaster();
  int GetMinimumHeartbeatMillis() const;
  int GetMillisUntilNextHeartbeat() const;
  Status DoHeartbeat();
  Status SetupRegistration(ServerRegistrationPB* reg);
  void SetupCommonField(master::WorkerToMasterCommonPB* common);
  bool IsCurrentThread() const;

  HostPort master_address_;
  WorkerServer* const server_;
  scoped_refptr<mprmpr::Thread> thread_;
  gscoped_ptr<master::MasterServiceProxy> proxy_;
  master::WorkerHeartbeatResponsePB last_hb_response_;
  int consecutive_failed_heartbeats_;

  struct WorkerReportState {
    int32_t change_seq;
  };
  typedef std::unordered_map<std::string, WorkerReportState> DirtyMap;
  
  DirtyMap dirty_workers_;
  
  mutable simple_spinlock dirty_workers_lock_;
  
  std::atomic_int next_report_seq_;
  
  Mutex mutex_;
  ConditionVariable cond_;
  
  bool should_run_;
  bool heartbeat_asap_;
  
  bool send_full_worker_report_;
  
  DISALLOW_COPY_AND_ASSIGN(Thread);
};


// Heartbeater

Heartbeater::Heartbeater(const WorkerServerOptions& opts, WorkerServer* server) {
  DCHECK_GT(opts.master_addresses.size(), 0);

  for (const auto& addr : opts.master_addresses) {
    threads_.emplace_back(new Thread(addr, server));
  }
}
Heartbeater::~Heartbeater() {
  WARN_NOT_OK(Stop(), "Unable to stop heartbeater thread");
}

Status Heartbeater::Start() {
  for (int i = 0; i < threads_.size(); i++) {
    Status first_failure = threads_[i]->Start();
    if (!first_failure.ok()) {
      for (int j = 0; j < i; j++) {
        threads_[j]->Stop();
      }
      return first_failure;
    }
  }

  return Status::OK();
}

Status Heartbeater::Stop() {
  Status first_failure;
  for (const auto& thread : threads_) {
    Status s = thread->Stop();
    if (!s.ok() && first_failure.ok()) {
      first_failure = s;
    } 
  } 
  return first_failure;
} 

void Heartbeater::TriggerASAP() {
  for (const auto& thread : threads_) {
    thread->TriggerASAP();
  } 
} 

vector<WorkerReportPB> Heartbeater::GenerateIncrementalWorkerReportsForTests() {
  vector<WorkerReportPB> results;
  for (const auto& thread : threads_) {
    WorkerReportPB report;
    thread->GenerateIncrementWorkerReport(&report);
    results.emplace_back(std::move(report));
  }
  return results;
}

vector<WorkerReportPB> Heartbeater::GenerateFullWorkerReportsForTests() {
  vector<WorkerReportPB>  results;
  for (const auto& thread : threads_) {
    WorkerReportPB report;
    thread->GenerateFullWorkerReport(&report);
    results.emplace_back(std::move(report));
  }
  return results;
}

void Heartbeater::MarkWorkerReportsAcknowledgedForTests(
    const vector<WorkerReportPB>& reports) {
  CHECK_EQ(reports.size(), threads_.size());

  for (int i = 0; i < reports.size(); i++) {
    threads_[i]->MarkWorkerReportAcknowledged(reports[i]);
  }
}


// Hearbeater::Thread
Heartbeater::Thread::Thread(const HostPort& master_address, WorkerServer* server)
  : master_address_(master_address),
    server_(server),
    consecutive_failed_heartbeats_(0),
    next_report_seq_(0),
    cond_(&mutex_),
    should_run_(false),
    heartbeat_asap_(true),
    send_full_worker_report_(false) {
}

Status Heartbeater::Thread::ConnectToMaster() {
  gscoped_ptr<MasterServiceProxy> new_proxy;
  RETURN_NOT_OK(MasterServiceProxyForHostPort(master_address_, server_->messenger(), &new_proxy));

  master::PingRequestPB req;
  master::PingResponsePB resp;
  RpcController rpc;
  rpc.set_timeout(MonoDelta::FromMilliseconds(FLAGS_heartbeat_rpc_timeout_ms));
  RETURN_NOT_OK_PREPEND(new_proxy->Ping(req, &resp, &rpc),
                                        Substitute("Failed to ping master at $0", master_address_.ToString()));
  LOG(INFO) << "Connected to a master server at " << master_address_.ToString();
  proxy_.reset(new_proxy.release());
  return Status::OK();
}

void Heartbeater::Thread::SetupCommonField(master::WorkerToMasterCommonPB* common) {
  common->mutable_worker_instance()->CopyFrom(server_->instance_pb());
}

Status Heartbeater::Thread::SetupRegistration(ServerRegistrationPB* reg) {
  reg->Clear();

  vector<Sockaddr> addrs;
  RETURN_NOT_OK(CHECK_NOTNULL(server_->rpc_server())->GetBoundAddresses(&addrs));
  RETURN_NOT_OK_PREPEND(AddHostPortPBs(addrs, reg->mutable_rpc_addresses()),
                                       "Failed to add RPC addresses to registration");

  addrs.clear();
  RETURN_NOT_OK_PREPEND(CHECK_NOTNULL(server_->web_server())->GetBoundAddresses(&addrs),
                                      "Unable to get bound HTTP addresses");
  RETURN_NOT_OK_PREPEND(AddHostPortPBs(addrs, reg->mutable_http_addresses()),
                                       "Failed to add HTTP addresses to registration");
  reg->set_software_version(VersionInfo::GetShortVersionString());

  return Status::OK();
}

int Heartbeater::Thread::GetMinimumHeartbeatMillis() const {
  if (consecutive_failed_heartbeats_ == FLAGS_heartbeat_max_failures_before_backoff) {
    LOG(WARNING) << "Failed " << consecutive_failed_heartbeats_  <<" heartbeats "
                 << "in a row: no longer allowing fast heartbeat attempts.";
  }

  return consecutive_failed_heartbeats_ >= FLAGS_heartbeat_max_failures_before_backoff ?
    FLAGS_heartbeat_interval_ms : 0;
}

int Heartbeater::Thread::GetMillisUntilNextHeartbeat() const {
  if (last_hb_response_.needs_reregister() ||
            last_hb_response_.needs_full_worker_report()) {
    return GetMinimumHeartbeatMillis();
  } 
  
  return FLAGS_heartbeat_interval_ms;
} 

Status Heartbeater::Thread::DoHeartbeat() {
  if (PREDICT_FALSE(server_->fail_heartbeats_for_tests())) {
    return Status::IOError("failing all heartbeats for tests");
  } 
  
  CHECK(IsCurrentThread());
  
  if (!proxy_) {
    VLOG(1) << "No valid master proxy. Connecting...";
    RETURN_NOT_OK(ConnectToMaster());
    DCHECK(proxy_);
  } 
  
  master::WorkerHeartbeatRequestPB req;
  
  SetupCommonField(req.mutable_common());
  if (last_hb_response_.needs_reregister()) {
    LOG(INFO) << "Registering TS with master...";
    RETURN_NOT_OK_PREPEND(SetupRegistration(req.mutable_registration()),
                          "Unable to set up registration");
  }                       
  
  if (send_full_worker_report_) {
    LOG(INFO) << Substitute(
        "Master $0 was elected leader, sending a full worker report...",
        master_address_.ToString());
    GenerateFullWorkerReport(req.mutable_worker_report());
  } else if (last_hb_response_.needs_full_worker_report()) {
    LOG(INFO) << Substitute(
        "Master $0 requested a full worker report, sending...",
        master_address_.ToString());
    GenerateFullWorkerReport(req.mutable_worker_report());
  } else {
    VLOG(2) << Substitute("Sending an incremental worker report to master $0...",
                          master_address_.ToString());
    GenerateIncrementWorkerReport(req.mutable_worker_report());
  }
  //req.set_num_live_tablets(server_->worker_manager()->GetNumLiveWorkers());

  RpcController rpc;
  rpc.set_timeout(MonoDelta::FromMilliseconds(FLAGS_heartbeat_rpc_timeout_ms));

  VLOG(2) << "Sending heartbeat:\n" << req.DebugString();
  master::WorkerHeartbeatResponsePB resp;
  RETURN_NOT_OK_PREPEND(proxy_->WorkerHeartbeat(req, &resp, &rpc),
                                                "Failed to send heartbeat to master");
  if (resp.has_error()) {
    return StatusFromPB(resp.error().status());
  }



  VLOG(2) << Substitute("Received heartbeat response from $0:\n$1",
                                                master_address_.ToString(), resp.DebugString());

#if 0
  if (!last_hb_response_.leader_master() && resp.leader_master()) {
    send_full_worker_report_ = true;
  } else {
    send_full_worker_report_ = false;
  }
#endif
  send_full_worker_report_ = true;

  last_hb_response_.Swap(&resp);

  MarkWorkerReportAcknowledged(req.worker_report());
  return Status::OK();
}

void Heartbeater::Thread::RunThread() {
  CHECK(IsCurrentThread());
  VLOG(1) << Substitute("Heartbeat thread (master $0) starting",
                        master_address_.ToString());

  last_hb_response_.set_needs_reregister(true);
  last_hb_response_.set_needs_full_worker_report(true);

  while (true) {
    MonoTime next_heartbeat =
        MonoTime::Now() + MonoDelta::FromMilliseconds(GetMillisUntilNextHeartbeat());

    {
      MutexLock l(mutex_);
      while (true) {
        MonoDelta remaining = next_heartbeat - MonoTime::Now();
        if (remaining.ToMilliseconds() <= 0 ||
                        heartbeat_asap_ ||
                        !should_run_) {
          break;
        }
        cond_.TimedWait(remaining);
      }

      heartbeat_asap_ = false;

      if (!should_run_) {
        VLOG(1) << Substitute("Heartbeat thread (master $0) finished",
                              master_address_.ToString());
        return;
      }
    }

    Status s = DoHeartbeat();
    if (!s.ok()) {
      LOG(WARNING) << Substitute("Failed to heartbeat to $0: $1",
                                 master_address_.ToString(), s.ToString());
      consecutive_failed_heartbeats_++;
      if (s.IsNetworkError() ||
                    consecutive_failed_heartbeats_ >= FLAGS_heartbeat_max_failures_before_backoff) {
        proxy_.reset();
      }
      continue;
    }
    consecutive_failed_heartbeats_ = 0;
  }
}

bool Heartbeater::Thread::IsCurrentThread() const {
  return thread_.get() == mprmpr::Thread::current_thread();
}

void Heartbeater::Thread::MarkWorkerReportAcknowledged(const master::WorkerReportPB& report) {

  std::lock_guard<simple_spinlock> l(dirty_workers_lock_);

  int32_t acked_seq = report.sequence_number();
  CHECK_LT(acked_seq, next_report_seq_.load());

  auto it = dirty_workers_.begin();
  while (it != dirty_workers_.end()) {
    const WorkerReportState& state = it->second;
    if (state.change_seq <= acked_seq) {
      it = dirty_workers_.erase(it);
    } else {
      ++it;
    }
  }

}

Status Heartbeater::Thread::Start() {
  CHECK(thread_ == nullptr);

  should_run_ = true;
  return mprmpr::Thread::Create("heartbeater", "heartbeat",
                             &Heartbeater::Thread::RunThread, this, &thread_);
}

Status Heartbeater::Thread::Stop() {
  if (!thread_) {
    return Status::OK();
  }

  {
    MutexLock l(mutex_);
    should_run_ = false;
    cond_.Signal();
  }
  RETURN_NOT_OK(ThreadJoiner(thread_.get()).Join());
  thread_ = nullptr;
  return Status::OK();
}

void Heartbeater::Thread::TriggerASAP() {
  MutexLock l(mutex_);
  heartbeat_asap_ = true;
  cond_.Signal();
}

void Heartbeater::Thread::GenerateIncrementWorkerReport(WorkerReportPB* report) {
  report->Clear();
  report->set_sequence_number(next_report_seq_.fetch_add(1));
  report->set_is_incremental(true);
  report->set_msg("Increment Worker Report");

#if 0
  vector<string> dirty_tablet_ids;
  {
    std::lock_guard<simple_spinlock> l(dirty_tablets_lock_);
    AppendKeysFromMap(dirty_tablets_, &dirty_tablet_ids);
  }
  server_->tablet_manager()->PopulateIncrementalWorkerReport(
      report, dirty_tablet_ids);
#endif
}

void Heartbeater::Thread::GenerateFullWorkerReport(WorkerReportPB* report) {
  report->Clear();
  report->set_sequence_number(next_report_seq_.fetch_add(1));
  report->set_is_incremental(false);
  report->set_msg("Full Worker Report");
//  server_->tablet_manager()->PopulateFullWorkerReport(report);
}

} // namespace worker_server
} // namespace mprmpr

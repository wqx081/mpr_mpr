#include "mprmpr/master/master.h"

#include <algorithm>
#include <boost/bind.hpp>
#include <glog/logging.h>
#include <list>
#include <memory>
#include <vector>

#include "mprmpr/common/common.h"
#include "mprmpr/common/version_info.h"

#include "mprmpr/base/strings/substitute.h"
#include "mprmpr/master/master_service_impl.h"
#include "mprmpr/master/master.proxy.pb.h"
#include "mprmpr/master/master_path_handlers.h"
#include "mprmpr/master/worker_manager.h"
#include "mprmpr/rpc/messenger.h"
#include "mprmpr/rpc/service_if.h"
#include "mprmpr/rpc/service_pool.h"
#include "mprmpr/server/rpc_server.h"
#include "mprmpr/util/net/net_util.h"
#include "mprmpr/util/net/sockaddr.h"
#include "mprmpr/util/status.h"
#include "mprmpr/util/threadpool.h"


namespace mprmpr {
namespace master {

Master::Master(const MasterOptions& options)
    : ServerBase("Master", options, "mprmpr.master"),
      state_(kStopped),
      worker_manager_(new WorkerManager()),
      options_(options),
      registration_initialized_(false) {
}

Master::~Master() {
  CHECK_NE(kRunning, state_);
}

std::string Master::ToString() const {
  if (state_ != kRunning) {
    return "Master (stopped)";
  }
  return strings::Substitute("Master@$0", first_rpc_address().ToString());
}

Status Master::Init() {
  CHECK_EQ(kStopped, state_);

  RETURN_NOT_OK(ServerBase::Init());
  // TODO: wqx
  // add web server path handlers
  
  state_ = kInitialized;
  return Status::OK();
}

Status Master::Start() {
  RETURN_NOT_OK(StartAsync());
  google::FlushLogFiles(google::INFO);
  return Status::OK();
}

Status Master::StartAsync() {
  CHECK_EQ(kInitialized, state_);

  gscoped_ptr<rpc::ServiceIf> impl(new MasterServiceImpl(this));

  RETURN_NOT_OK(ServerBase::RegisterService(std::move(impl)));
  RETURN_NOT_OK(ServerBase::Start());

  RETURN_NOT_OK(InitMasterRegistration());
  state_ = kRunning;

  return Status::OK();
}

void Master::Shutdown() {
  if (state_ == kRunning) {
    std::string name = ToString();
    LOG(INFO) << name << " shutting down...";
    ServerBase::Shutdown();
    LOG(INFO) << name << " shutdown complete.";
  }
  state_ = kStopped;
}

Status Master::GetMasterRegistration(ServerRegistrationPB* reg) const {
  if (!registration_initialized_.load(std::memory_order_acquire)) {
    return Status::ServiceUnavailable("Master startup not complete");
  }
  reg->CopyFrom(registration_);
  return Status::OK();
}

Status Master::InitMasterRegistration() {
  CHECK(!registration_initialized_.load());

  ServerRegistrationPB reg;
  std::vector<Sockaddr> rpc_addrs;
  RETURN_NOT_OK_PREPEND(rpc_server()->GetBoundAddresses(&rpc_addrs),
                        "Couldn't get RPC addresses");
  RETURN_NOT_OK(AddHostPortPBs(rpc_addrs, reg.mutable_rpc_addresses()));
  reg.set_software_version(VersionInfo::GetShortVersionString());

  registration_.Swap(&reg);
  registration_initialized_.store(true);

  return Status::OK();
}


} // namespace master
} // namespace mprmpr

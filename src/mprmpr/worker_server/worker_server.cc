#include "mprmpr/worker_server/worker_server.h"

#include <glog/logging.h>
#include <list>
#include <vector>

#include "mprmpr/base/strings/substitute.h"
#include "mprmpr/rpc/service_if.h"
#include "mprmpr/server/rpc_server.h"
#include "mprmpr/server/web_server.h"

#include "mprmpr/worker_server/heartbeater.h"
//#include "mprmpr/worker_server/worker_service.h"
//
//
#include "mprmpr/util/net/net_util.h"
#include "mprmpr/util/net/sockaddr.h"
#include "mprmpr/util/status.h"

using mprmpr::rpc::ServiceIf;
using std::shared_ptr;
using std::vector;

namespace mprmpr {
namespace worker_server {

WorkerServer::WorkerServer(const WorkerServerOptions& opts)
  : ServerBase("WorkerServer", opts, "mprmpr.workerserver"),
    initted_(false),
    fail_heartbeats_for_tests_(false),
    opts_(opts) {
}

WorkerServer::~WorkerServer() {
  Shutdown();
}

string WorkerServer::ToString() const {
  return "WorkerServer";
}

Status WorkerServer::ValidateMasterAddressResolution() const {
  for (const HostPort& master_addr : opts_.master_addresses) {
    RETURN_NOT_OK_PREPEND(master_addr.ResolveAddresses(NULL),
                          strings::Substitute(
                              "Couldn't resolve master service address '$0'",
                              master_addr.ToString()));
  }
  return Status::OK();
}

Status WorkerServer::Init() {
  CHECK(!initted_);

  RETURN_NOT_OK(ValidateMasterAddressResolution());

  RETURN_NOT_OK(ServerBase::Init());

  heartbeater_.reset(new Heartbeater(opts_, this));

  initted_ = true;
  return Status::OK();
}

Status WorkerServer::WaitInited() {
//  return tablet_manager_->WaitForAllBootstrapsToFinish();
  return Status::OK();
}

Status WorkerServer::Start() {
  CHECK(initted_);

  RETURN_NOT_OK(ServerBase::Start());

  RETURN_NOT_OK(heartbeater_->Start());

  google::FlushLogFiles(google::INFO); // Flush the startup messages.

  return Status::OK();
}

void WorkerServer::Shutdown() {
  LOG(INFO) << "WorkerServer shutting down...";

  if (initted_) {
    WARN_NOT_OK(heartbeater_->Stop(), "Failed to stop TS Heartbeat thread");
    ServerBase::Shutdown();
  }

  LOG(INFO) << "WorkerServer shut down complete. Bye!";
}
} // namespace worker_server

} // namespace mprmpr

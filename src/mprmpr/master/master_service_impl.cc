#include "mprmpr/master/master_service_impl.h"

#include <gflags/gflags.h>
#include <memory>
#include <string>
#include <vector>

#include "mprmpr/common/common.h"
#include "mprmpr/master/master.h"
#include "mprmpr/master/worker_descriptor.h"
#include "mprmpr/master/worker_manager.h"
#include "mprmpr/rpc/rpc_context.h"
#include "mprmpr/server/web_server.h"
#include "mprmpr/base/strings/substitute.h"

namespace mprmpr {
namespace master { 
                  
MasterServiceImpl::MasterServiceImpl(Master* master)
    : MasterServiceIf(master->metric_entity(), master->result_tracker()),
      server_(master) {
}

void MasterServiceImpl::Ping(const PingRequestPB* req,
                             PingResponsePB* resp,
                             rpc::RpcContext* rpc) {
  rpc->RespondSuccess();
}

void MasterServiceImpl::WorkerHeartbeat(const WorkerHeartbeatRequestPB* req,
                                        WorkerHeartbeatResponsePB* resp,
                                        rpc::RpcContext* rpc) {
  //TODO(wqx):
  
  std::shared_ptr<WorkerDescriptor> desc;
  if (req->has_registration()) {
    Status s = server_->worker_manager()->RegisterWorker(req->common().worker_instance(),
                                                         req->registration(),
                                                         req->worker_status(),
                                                         &desc);
    if (!s.ok()) {
      LOG(WARNING) << strings::Substitute("Unable to register worker server ($0): $1",
                                          rpc->requestor_string(), s.ToString());
      rpc->RespondFailure(s);
      return;
    }
  } else {
    Status s = server_->worker_manager()->LookupWorker(req->common().worker_instance(), &desc);
    if (s.IsNotFound()) {
      LOG(INFO) << strings::Substitute("Got hearbeat from unknown worker server ($0) as $1; "
                                       "Asking this server to re-register.",
                                       req->common().worker_instance().ShortDebugString(),
                                       rpc->requestor_string());
      resp->set_needs_register(true);
      rpc->RespondSuccess();
      return;
    } else if (!s.ok()) {
      LOG(WARNING) << strings::Substitute("Unable to look up worker server for heartbeat request $0 from $1: $2",
                                          req->DebugString(),
                                          rpc->requestor_string(),
                                          s.ToString());
      rpc->RespondFailure(s.CloneAndPrepend("Unable to lookup worker server"));
      return;
    }
  }

  desc->UpdateHearbeatTime();
  desc->UpdateWorkerStatus(req->worker_status());

  //TODO(wqx):
  LOG(INFO) << "---------- Handle worker server report";

  rpc->RespondSuccess();
}

} // namespace master
} // namespace mprmpr

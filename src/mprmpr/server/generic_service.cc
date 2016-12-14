#include "mprmpr/server/generic_service.h"

#include <gflags/gflags.h>
#include <string>
#include <unordered_set>

#include "mprmpr/base/map-util.h"
#include "mprmpr/rpc/rpc_context.h"
#include "mprmpr/server/clock.h"
#include "mprmpr/server/hybrid_clock.h"
#include "mprmpr/server/server_base.h"
#include "mprmpr/util/debug-util.h"
//#include "mprmpr/util/flag_tags.h"

DECLARE_bool(use_mock_wall_clock);
DECLARE_bool(use_hybrid_clock);

using std::string;
using std::unordered_set;

namespace mprmpr {
namespace server {

GenericServiceImpl::GenericServiceImpl(ServerBase* server)
  : GenericServiceIf(server->metric_entity(), server->result_tracker()),
    server_(server) {
}

GenericServiceImpl::~GenericServiceImpl() {
}

void GenericServiceImpl::SetFlag(const SetFlagRequestPB* req,
                                 SetFlagResponsePB* resp,
                                 rpc::RpcContext* rpc) {

  // Validate that the flag exists and get the current value.
  string old_val;
  if (!google::GetCommandLineOption(req->flag().c_str(),
                                    &old_val)) {
    resp->set_result(SetFlagResponsePB::NO_SUCH_FLAG);
    rpc->RespondSuccess();
    return;
  }

  // Validate that the flag is runtime-changeable.
  unordered_set<string> tags;
//TODO(wqx):
//  GetFlagTags(req->flag(), &tags);
  if (!ContainsKey(tags, "runtime")) {
    if (req->force()) {
      LOG(WARNING) << rpc->requestor_string() << " forcing change of "
                   << "non-runtime-safe flag " << req->flag();
    } else {
      resp->set_result(SetFlagResponsePB::NOT_SAFE);
      resp->set_msg("Flag is not safe to change at runtime");
      rpc->RespondSuccess();
      return;
    }
  }

  resp->set_old_value(old_val);

  // Try to set the new value.
  string ret = google::SetCommandLineOption(
      req->flag().c_str(),
      req->value().c_str());
  if (ret.empty()) {
    resp->set_result(SetFlagResponsePB::BAD_VALUE);
    resp->set_msg("Unable to set flag: bad value");
  } else {
    LOG(INFO) << rpc->requestor_string() << " changed flags via RPC: "
              << req->flag() << " from '" << old_val << "' to '"
              << req->value() << "'";
    resp->set_result(SetFlagResponsePB::SUCCESS);
    resp->set_msg(ret);
  }

  rpc->RespondSuccess();
}

void GenericServiceImpl::FlushCoverage(const FlushCoverageRequestPB* req,
                                       FlushCoverageResponsePB* resp,
                                       rpc::RpcContext* rpc) {
  if (IsCoverageBuild()) {
    TryFlushCoverage();
    LOG(INFO) << "Flushed coverage info. (request from " << rpc->requestor_string() << ")";
    resp->set_success(true);
  } else {
    LOG(WARNING) << "Non-coverage build cannot flush coverage (request from "
                 << rpc->requestor_string() << ")";
    resp->set_success(false);
  }
  rpc->RespondSuccess();
}

void GenericServiceImpl::ServerClock(const ServerClockRequestPB* req,
                                     ServerClockResponsePB* resp,
                                     rpc::RpcContext* rpc) {
  resp->set_timestamp(server_->clock()->Now().ToUint64());
  rpc->RespondSuccess();
}

void GenericServiceImpl::SetServerWallClockForTests(const SetServerWallClockForTestsRequestPB *req,
                                                   SetServerWallClockForTestsResponsePB *resp,
                                                   rpc::RpcContext *context) {
  if (!FLAGS_use_hybrid_clock || !FLAGS_use_mock_wall_clock) {
    LOG(WARNING) << "Error setting wall clock for tests. Server is not using HybridClock"
        "or was not started with '--use_mock_wall_clock= true'";
    resp->set_success(false);
  }

  server::HybridClock* clock = down_cast<server::HybridClock*>(server_->clock());
  if (req->has_now_usec()) {
    clock->SetMockClockWallTimeForTests(req->now_usec());
  }
  if (req->has_max_error_usec()) {
    clock->SetMockMaxClockErrorForTests(req->max_error_usec());
  }
  resp->set_success(true);
  context->RespondSuccess();
}

void GenericServiceImpl::GetStatus(const GetStatusRequestPB* req,
                                   GetStatusResponsePB* resp,
                                   rpc::RpcContext* rpc) {
  server_->GetStatusPB(resp->mutable_status());
  rpc->RespondSuccess();
}

} // namespace server
} // namespace mprmpr

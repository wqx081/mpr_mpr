#include "mprmpr/rpc/rpc.h"

#include <functional>
#include <string>

#include "mprmpr/base/strings/substitute.h"
#include "mprmpr/rpc/messenger.h"
#include "mprmpr/rpc/rpc_header.pb.h"

using std::shared_ptr;
using strings::Substitute;
using strings::SubstituteAndAppend;

namespace mprmpr {

namespace rpc {

bool RpcRetrier::HandleResponse(Rpc* rpc, Status* out_status) {
  DCHECK(rpc);
  DCHECK(out_status);

  // Always retry a TOO_BUSY error.
  Status controller_status = controller_.status();
  if (controller_status.IsRemoteError()) {
    const ErrorStatusPB* err = controller_.error_response();
    if (err &&
        err->has_code() &&
        err->code() == ErrorStatusPB::ERROR_SERVER_TOO_BUSY) {
      DelayedRetry(rpc, controller_status);
      return true;
    }
  }

  *out_status = controller_status;
  return false;
}

void RpcRetrier::DelayedRetry(Rpc* rpc, const Status& why_status) {
  if (!why_status.ok() && (last_error_.ok() || last_error_.IsTimedOut())) {
    last_error_ = why_status;
  }
  // Add some jitter to the retry delay.
  //
  // If the delay causes us to miss our deadline, RetryCb will fail the
  // RPC on our behalf.
  int num_ms = ++attempt_num_ + ((rand() % 5));
  messenger_->ScheduleOnReactor(std::bind(&RpcRetrier::DelayedRetryCb,
                                            this,
                                            rpc, std::placeholders::_1),
                                MonoDelta::FromMilliseconds(num_ms));
}

void RpcRetrier::DelayedRetryCb(Rpc* rpc, const Status& status) {
  Status new_status = status;
  if (new_status.ok()) {
    // Has this RPC timed out?
    if (deadline_.Initialized()) {
      if (MonoTime::Now() > deadline_) {
        string err_str = Substitute("$0 passed its deadline", rpc->ToString());
        if (!last_error_.ok()) {
          SubstituteAndAppend(&err_str, ": $0", last_error_.ToString());
        }
        new_status = Status::TimedOut(err_str);
      }
    }
  }
  if (new_status.ok()) {
    controller_.Reset();
    rpc->SendRpc();
  } else {
    rpc->SendRpcCb(new_status);
  }
}

} // namespace rpc
} // namespace mprmpr

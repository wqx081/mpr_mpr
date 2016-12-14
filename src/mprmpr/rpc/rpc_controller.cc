#include "mprmpr/rpc/rpc_controller.h"

#include <algorithm>
#include <glog/logging.h>
#include <mutex>

#include "mprmpr/rpc/rpc_header.pb.h"
#include "mprmpr/rpc/outbound_call.h"

namespace mprmpr { namespace rpc {

RpcController::RpcController() {
  DVLOG(4) << "RpcController " << this << " constructed";
}

RpcController::~RpcController() {
  DVLOG(4) << "RpcController " << this << " destroyed";
}

void RpcController::Swap(RpcController* other) {
  // Cannot swap RPC controllers while they are in-flight.
  if (call_) {
    CHECK(finished());
  }
  if (other->call_) {
    CHECK(other->finished());
  }

  std::swap(timeout_, other->timeout_);
  std::swap(call_, other->call_);
}

void RpcController::Reset() {
  std::lock_guard<simple_spinlock> l(lock_);
  if (call_) {
    CHECK(finished());
  }
  call_.reset();
}

bool RpcController::finished() const {
  if (call_) {
    return call_->IsFinished();
  }
  return false;
}

Status RpcController::status() const {
  if (call_) {
    return call_->status();
  }
  return Status::OK();
}

const ErrorStatusPB* RpcController::error_response() const {
  if (call_) {
    return call_->error_pb();
  }
  return nullptr;
}

Status RpcController::GetSidecar(int idx, Slice* sidecar) const {
  return call_->call_response_->GetSidecar(idx, sidecar);
}

void RpcController::set_timeout(const MonoDelta& timeout) {
  std::lock_guard<simple_spinlock> l(lock_);
  DCHECK(!call_ || call_->state() == OutboundCall::READY);
  timeout_ = timeout;
}

void RpcController::set_deadline(const MonoTime& deadline) {
  set_timeout(deadline - MonoTime::Now());
}

void RpcController::SetRequestIdPB(std::unique_ptr<RequestIdPB> request_id) {
  request_id_ = std::move(request_id);
}

bool RpcController::has_request_id() const {
  return request_id_ != nullptr;
}

const RequestIdPB& RpcController::request_id() const {
  DCHECK(has_request_id());
  return *request_id_;
}

void RpcController::RequireServerFeature(uint32_t feature) {
  DCHECK(!call_ || call_->state() == OutboundCall::READY);
  required_server_features_.insert(feature);
}

MonoDelta RpcController::timeout() const {
  std::lock_guard<simple_spinlock> l(lock_);
  return timeout_;
}

} // namespace rpc
} // namespace kudu

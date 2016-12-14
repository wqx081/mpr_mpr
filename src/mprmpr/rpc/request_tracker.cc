#include "mprmpr/rpc/request_tracker.h"

#include <mutex>

#include "mprmpr/base/map-util.h"

namespace mprmpr {
namespace rpc {

const RequestTracker::SequenceNumber RequestTracker::NO_SEQ_NO = -1;

RequestTracker::RequestTracker(const string& client_id)
    : client_id_(client_id),
      next_(0) {}

Status RequestTracker::NewSeqNo(SequenceNumber* seq_no) {
  std::lock_guard<simple_spinlock> l(lock_);
  *seq_no = next_;
  InsertOrDie(&incomplete_rpcs_, *seq_no);
  next_++;
  return Status::OK();
}

RequestTracker::SequenceNumber RequestTracker::FirstIncomplete() {
  std::lock_guard<simple_spinlock> l(lock_);
  if (incomplete_rpcs_.empty()) return NO_SEQ_NO;
  return *incomplete_rpcs_.begin();
}

void RequestTracker::RpcCompleted(const SequenceNumber& seq_no) {
  std::lock_guard<simple_spinlock> l(lock_);
  incomplete_rpcs_.erase(seq_no);
}

} // namespace rpc
} // namespace mprmpr

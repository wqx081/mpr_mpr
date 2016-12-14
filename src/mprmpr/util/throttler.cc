#include "mprmpr/util/throttler.h"

#include <mutex>

namespace mprmpr {

Throttler::Throttler(MonoTime now, uint64_t op_rate, uint64_t byte_rate, double burst_factor) :
    next_refill_(now) {
  op_refill_ = op_rate / (MonoTime::kMicrosecondsPerSecond / kRefillPeriodMicros);
  op_token_ = 0;
  op_token_max_ = static_cast<uint64_t>(op_refill_ * burst_factor);
  byte_refill_ = byte_rate / (MonoTime::kMicrosecondsPerSecond / kRefillPeriodMicros);
  byte_token_ = 0;
  byte_token_max_ = static_cast<uint64_t>(byte_refill_ * burst_factor);
}

bool Throttler::Take(MonoTime now, uint64_t op, uint64_t byte) {
  if (op_refill_ == 0 && byte_refill_ == 0) {
    return true;
  }
  std::lock_guard<simple_spinlock> lock(lock_);
  Refill(now);
  if ((op_refill_ == 0 || op <= op_token_) &&
      (byte_refill_ == 0 || byte <= byte_token_)) {
    if (op_refill_ > 0) {
      op_token_ -= op;
    }
    if (byte_refill_ > 0) {
      byte_token_ -= byte;
    }
    return true;
  }
  return false;
}

void Throttler::Refill(MonoTime now) {
  int64_t d = (now - next_refill_).ToMicroseconds();
  if (d < 0) {
    return;
  }
  uint64_t num_period = d / kRefillPeriodMicros + 1;
  next_refill_ += MonoDelta::FromMicroseconds(num_period * kRefillPeriodMicros);
  op_token_ += num_period * op_refill_;
  op_token_ = std::min(op_token_, op_token_max_);
  byte_token_ += num_period * byte_refill_;
  byte_token_ = std::min(byte_token_, byte_token_max_);
}

} // namespace mprmpr

#ifndef KUDU_UTIL_THROTTLER_H
#define KUDU_UTIL_THROTTLER_H

#include <algorithm>

#include "mprmpr/base/macros.h"
#include "mprmpr/util/locks.h"
#include "mprmpr/util/monotime.h"

namespace mprmpr {

// A throttler to throttle both operation/s and IO byte/s.
class Throttler {
 public:
  // Refill period is 100ms.
  enum {
    kRefillPeriodMicros = 100000
  };

  // Construct a throttler with max operation per second, max IO bytes per second
  // and burst factor (burst_rate = rate * burst), burst rate means maximum
  // throughput within one refill period.
  // Set op_per_sec to 0 to disable operation throttling.
  // Set byte_per_sec to 0 to disable IO bytes throttling.
  Throttler(MonoTime now, uint64_t op_per_sec, uint64_t byte_per_sec, double burst_factor);

  // Throttle an "operation group" by taking 'op' operation tokens and 'byte' byte tokens.
  // Return true if there are enough tokens, and operation is allowed.
  // Return false if there are not enough tokens, and operation is throttled.
  bool Take(MonoTime now, uint64_t op, uint64_t byte);

 private:
  void Refill(MonoTime now);

  MonoTime next_refill_;
  uint64_t op_refill_;
  uint64_t op_token_;
  uint64_t op_token_max_;
  uint64_t byte_refill_;
  uint64_t byte_token_;
  uint64_t byte_token_max_;
  simple_spinlock lock_;
};

} // namespace mprmpr

#endif

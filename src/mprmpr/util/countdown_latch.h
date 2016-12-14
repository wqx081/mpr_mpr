#ifndef KUDU_UTIL_COUNTDOWN_LATCH_H
#define KUDU_UTIL_COUNTDOWN_LATCH_H

#include "mprmpr/base/macros.h"
#include "mprmpr/util/condition_variable.h"
#include "mprmpr/util/monotime.h"
#include "mprmpr/util/mutex.h"

namespace mprmpr {

class CountDownLatch {
 public:
  // Initialize the latch with the given initial count.
  explicit CountDownLatch(int count)
    : cond_(&lock_),
      count_(count) {
  }

  // Decrement the count of this latch by 'amount'
  // If the new count is less than or equal to zero, then all waiting threads are woken up.
  // If the count is already zero, this has no effect.
  void CountDown(int amount) {
    DCHECK_GE(amount, 0);
    MutexLock lock(lock_);
    if (count_ == 0) {
      return;
    }

    if ((unsigned int)amount >= count_) {
      count_ = 0;
    } else {
      count_ -= amount;
    }

    if (count_ == 0) {
      // Latch has triggered.
      cond_.Broadcast();
    }
  }

  // Decrement the count of this latch.
  // If the new count is zero, then all waiting threads are woken up.
  // If the count is already zero, this has no effect.
  void CountDown() {
    CountDown(1);
  }

  // Wait until the count on the latch reaches zero.
  // If the count is already zero, this returns immediately.
  void Wait() const {
//    ThreadRestrictions::AssertWaitAllowed();
    MutexLock lock(lock_);
    while (count_ > 0) {
      cond_.Wait();
    }
  }

  // Waits for the count on the latch to reach zero, or until 'until' time is reached.
  // Returns true if the count became zero, false otherwise.
  bool WaitUntil(const MonoTime& when) const {
//    ThreadRestrictions::AssertWaitAllowed();
    return WaitFor(when - MonoTime::Now());
  }

  // Waits for the count on the latch to reach zero, or until 'delta' time elapses.
  // Returns true if the count became zero, false otherwise.
  bool WaitFor(const MonoDelta& delta) const {
//    ThreadRestrictions::AssertWaitAllowed();
    MutexLock lock(lock_);
    while (count_ > 0) {
      if (!cond_.TimedWait(delta)) {
        return false;
      }
    }
    return true;
  }

  // Reset the latch with the given count. This is equivalent to reconstructing
  // the latch. If 'count' is 0, and there are currently waiters, those waiters
  // will be triggered as if you counted down to 0.
  void Reset(uint64_t count) {
    MutexLock lock(lock_);
    count_ = count;
    if (count_ == 0) {
      // Awake any waiters if we reset to 0.
      cond_.Broadcast();
    }
  }

  uint64_t count() const {
    MutexLock lock(lock_);
    return count_;
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(CountDownLatch);
  mutable Mutex lock_;
  ConditionVariable cond_;

  uint64_t count_;
};

// Utility class which calls latch->CountDown() in its destructor.
class CountDownOnScopeExit {
 public:
  explicit CountDownOnScopeExit(CountDownLatch *latch) : latch_(latch) {}
  ~CountDownOnScopeExit() {
    latch_->CountDown();
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(CountDownOnScopeExit);

  CountDownLatch *latch_;
};

} // namespace mprmpr
#endif

#include <gtest/gtest.h>
#include <mutex>
#include <thread>
#include <vector>

#include "mprmpr/base/integral_types.h"
#include "mprmpr/util/atomic.h"
#include "mprmpr/util/locks.h"
#include "mprmpr/util/monotime.h"
#include "mprmpr/util/rw_mutex.h"
#include "mprmpr/util/test_util.h"

using std::lock_guard;
using std::thread;
using std::try_to_lock;
using std::unique_lock;
using std::vector;

namespace mprmpr {

class RWMutexTest : public KuduTest,
                    public ::testing::WithParamInterface<RWMutex::Priority> {
 public:
  RWMutexTest()
     : lock_(GetParam()) {
  }
 protected:
  RWMutex lock_;
};

// Instantiate every test for each kind of RWMutex priority.
INSTANTIATE_TEST_CASE_P(Priorities, RWMutexTest,
                        ::testing::Values(RWMutex::Priority::PREFER_READING,
                                          RWMutex::Priority::PREFER_WRITING));

// Multi-threaded test that tries to find deadlocks in the RWMutex wrapper.
TEST_P(RWMutexTest, TestDeadlocks) {
  uint64_t number_of_writes = 0;
  AtomicInt<uint64_t> number_of_reads(0);

  AtomicBool done(false);
  vector<thread> threads;

  // Start several blocking and non-blocking read-write workloads.
  for (int i = 0; i < 2; i++) {
    threads.emplace_back([&](){
      while (!done.Load()) {
        lock_guard<RWMutex> l(lock_);
        number_of_writes++;
      }
    });
    threads.emplace_back([&](){
      while (!done.Load()) {
        unique_lock<RWMutex> l(lock_, try_to_lock);
        if (l.owns_lock()) {
          number_of_writes++;
        }
      }
    });
  }

  // Start several blocking and non-blocking read-only workloads.
  for (int i = 0; i < 2; i++) {
    threads.emplace_back([&](){
      while (!done.Load()) {
        shared_lock<RWMutex> l(lock_);
        number_of_reads.Increment();
      }
    });
    threads.emplace_back([&](){
      while (!done.Load()) {
        shared_lock<RWMutex> l(lock_, try_to_lock);
        if (l.owns_lock()) {
          number_of_reads.Increment();
        }
      }
    });
  }

  SleepFor(MonoDelta::FromSeconds(1));
  done.Store(true);
  for (auto& t : threads) {
    t.join();
  }

  shared_lock<RWMutex> l(lock_);
  LOG(INFO) << "Number of writes: " << number_of_writes;
  LOG(INFO) << "Number of reads: " << number_of_reads.Load();
}

#ifndef NDEBUG
// Tests that the RWMutex wrapper catches basic usage errors. This checking is
// only enabled in debug builds.
TEST_P(RWMutexTest, TestLockChecking) {
  EXPECT_DEATH({
    lock_.ReadLock();
    lock_.ReadLock();
  }, "already holding lock for reading");

  EXPECT_DEATH({
    CHECK(lock_.TryReadLock());
    CHECK(lock_.TryReadLock());
  }, "already holding lock for reading");

  EXPECT_DEATH({
    lock_.ReadLock();
    lock_.WriteLock();
  }, "already holding lock for reading");

  EXPECT_DEATH({
    CHECK(lock_.TryReadLock());
    CHECK(lock_.TryWriteLock());
  }, "already holding lock for reading");

  EXPECT_DEATH({
    lock_.WriteLock();
    lock_.ReadLock();
  }, "already holding lock for writing");

  EXPECT_DEATH({
    CHECK(lock_.TryWriteLock());
    CHECK(lock_.TryReadLock());
  }, "already holding lock for writing");

  EXPECT_DEATH({
    lock_.WriteLock();
    lock_.WriteLock();
  }, "already holding lock for writing");

  EXPECT_DEATH({
    CHECK(lock_.TryWriteLock());
    CHECK(lock_.TryWriteLock());
  }, "already holding lock for writing");

  EXPECT_DEATH({
    lock_.ReadUnlock();
  }, "wasn't holding lock for reading");

  EXPECT_DEATH({
    lock_.WriteUnlock();
  }, "wasn't holding lock for writing");

  EXPECT_DEATH({
    lock_.ReadLock();
    lock_.WriteUnlock();
  }, "already holding lock for reading");

  EXPECT_DEATH({
    CHECK(lock_.TryReadLock());
    lock_.WriteUnlock();
  }, "already holding lock for reading");

  EXPECT_DEATH({
    lock_.WriteLock();
    lock_.ReadUnlock();
  }, "already holding lock for writing");

  EXPECT_DEATH({
    CHECK(lock_.TryWriteLock());
    lock_.ReadUnlock();
  }, "already holding lock for writing");
}
#endif

} // namespace mprmpr

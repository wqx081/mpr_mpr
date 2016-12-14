#include <glog/logging.h>
#include <gtest/gtest.h>
#include <memory>

#include "mprmpr/base/strings/substitute.h"
#include "mprmpr/util/monotime.h"
#include "mprmpr/util/striped64.h"
#include "mprmpr/util/test_util.h"
#include "mprmpr/util/thread.h"

// These flags are used by the multi-threaded tests, can be used for microbenchmarking.
DEFINE_int32(num_operations, 10*1000, "Number of operations to perform");
DEFINE_int32(num_threads, 2, "Number of worker threads");

namespace mprmpr {

// Test some basic operations
TEST(Striped64Test, TestBasic) {
  LongAdder adder;
  ASSERT_EQ(adder.Value(), 0);
  adder.IncrementBy(100);
  ASSERT_EQ(adder.Value(), 100);
  adder.Increment();
  ASSERT_EQ(adder.Value(), 101);
  adder.Decrement();
  ASSERT_EQ(adder.Value(), 100);
  adder.IncrementBy(-200);
  ASSERT_EQ(adder.Value(), -100);
  adder.Reset();
  ASSERT_EQ(adder.Value(), 0);
}

template <class Adder>
class MultiThreadTest {
 public:
  typedef std::vector<scoped_refptr<Thread> > thread_vec_t;

  MultiThreadTest(int64_t num_operations, int64_t num_threads)
   :  num_operations_(num_operations),
      num_threads_(num_threads) {
  }

  void IncrementerThread(const int64_t num) {
    for (int i = 0; i < num; i++) {
      adder_.Increment();
    }
  }

  void DecrementerThread(const int64_t num) {
    for (int i = 0; i < num; i++) {
      adder_.Decrement();
    }
  }

  void Run() {
    // Increment
    for (int i = 0; i < num_threads_; i++) {
      scoped_refptr<Thread> ref;
      Thread::Create("Striped64", "Incrementer", &MultiThreadTest::IncrementerThread, this,
                     num_operations_, &ref);
      threads_.push_back(ref);
    }
    for (const scoped_refptr<Thread> &t : threads_) {
      t->Join();
    }
    ASSERT_EQ(num_threads_*num_operations_, adder_.Value());
    threads_.clear();

    // Decrement back to zero
    for (int i = 0; i < num_threads_; i++) {
      scoped_refptr<Thread> ref;
      Thread::Create("Striped64", "Decrementer", &MultiThreadTest::DecrementerThread, this,
                     num_operations_, &ref);
      threads_.push_back(ref);
    }
    for (const scoped_refptr<Thread> &t : threads_) {
      t->Join();
    }
    ASSERT_EQ(0, adder_.Value());
  }

  Adder adder_;

  int64_t num_operations_;
  // This is rounded down to the nearest even number
  int32_t num_threads_;
  thread_vec_t threads_;
};

// Test adder implemented by a single AtomicInt for comparison
class BasicAdder {
 public:
  BasicAdder() : value_(0) {}
  void IncrementBy(int64_t x) { value_.IncrementBy(x); }
  inline void Increment() { IncrementBy(1); }
  inline void Decrement() { IncrementBy(-1); }
  int64_t Value() { return value_.Load(); }
 private:
  AtomicInt<int64_t> value_;
};

void RunMultiTest(int64_t num_operations, int64_t num_threads) {
  MonoTime start = MonoTime::Now();
  MultiThreadTest<BasicAdder> basicTest(num_operations, num_threads);
  basicTest.Run();
  MonoTime end1 = MonoTime::Now();
  MultiThreadTest<LongAdder> test(num_operations, num_threads);
  test.Run();
  MonoTime end2 = MonoTime::Now();
  MonoDelta basic = end1 - start;
  MonoDelta striped = end2 - end1;
  LOG(INFO) << "Basic counter took   " << basic.ToMilliseconds() << "ms.";
  LOG(INFO) << "Striped counter took " << striped.ToMilliseconds() << "ms.";
}

// Compare a single-thread workload. Demonstrates the overhead of LongAdder over AtomicInt.
TEST(Striped64Test, TestSingleIncrDecr) {
  OverrideFlagForSlowTests(
      "num_operations",
      strings::Substitute("$0", (FLAGS_num_operations * 100)));
  RunMultiTest(FLAGS_num_operations, 1);
}

// Compare a multi-threaded workload. LongAdder should show improvements here.
TEST(Striped64Test, TestMultiIncrDecr) {
  OverrideFlagForSlowTests(
      "num_operations",
      strings::Substitute("$0", (FLAGS_num_operations * 100)));
  OverrideFlagForSlowTests(
      "num_threads",
      strings::Substitute("$0", (FLAGS_num_threads * 4)));
  RunMultiTest(FLAGS_num_operations, FLAGS_num_threads);
}

}  // namespace mprmpr

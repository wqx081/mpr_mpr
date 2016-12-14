#include <gtest/gtest.h>
#include <glog/logging.h>
#include <strstream>

#include "mprmpr/base/spinlock.h"
#include "mprmpr/util/spinlock_profiling.h"
#include "mprmpr/util/test_util.h"
#include "mprmpr/util/trace.h"

// Can't include gutil/synchronization_profiling.h directly as it'll
// declare a weak symbol directly in this unit test, which the runtime
// linker will prefer over equivalent strong symbols for some reason. By
// declaring the symbol without providing an empty definition, the strong
// symbols are chosen when provided via shared libraries.
//
// Further reading:
// - http://stackoverflow.com/questions/20658809/dynamic-loading-and-weak-symbol-resolution
// - http://notmysock.org/blog/php/weak-symbols-arent.html
namespace gutil {
extern void SubmitSpinLockProfileData(const void *, int64);
} // namespace gutil

namespace base {
extern void SubmitSpinLockProfileData(const void *, int64);
} // namespace base

namespace mprmpr {

class SpinLockProfilingTest : public AntTest {};

TEST_F(SpinLockProfilingTest, TestSpinlockProfiling) {
  scoped_refptr<Trace> t(new Trace);
  base::SpinLock lock;
  {
    ADOPT_TRACE(t.get());
    gutil::SubmitSpinLockProfileData(&lock, 4000000);
  }
  string result = t->DumpToString();
  LOG(INFO) << "trace: " << result;
  ASSERT_STR_CONTAINS(result, "\"spinlock_wait_cycles\":4000000");
  // We can't assert more specifically because the CyclesPerSecond
  // on different machines might be different.
  ASSERT_STR_CONTAINS(result, "Waited ");
  ASSERT_STR_CONTAINS(result, "on lock ");

  ASSERT_GT(GetSpinLockContentionMicros(), 0);
}

TEST_F(SpinLockProfilingTest, TestStackCollection) {
  StartSynchronizationProfiling();
  base::SpinLock lock;
  gutil::SubmitSpinLockProfileData(&lock, 12345);
  StopSynchronizationProfiling();
  std::ostringstream str;
  int64_t dropped = 0;
  FlushSynchronizationProfile(&str, &dropped);
  string s = str.str();
  ASSERT_STR_CONTAINS(s, "12345\t1 @ ");
  ASSERT_EQ(0, dropped);
}

TEST_F(SpinLockProfilingTest, TestTcmallocContention) {
  StartSynchronizationProfiling();
  base::SubmitSpinLockProfileData(nullptr, 12345);
  ASSERT_GE(GetTcmallocContentionMicros(), 0);
}

} // namespace mprmpr

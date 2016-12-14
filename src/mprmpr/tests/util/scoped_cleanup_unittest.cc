#include "mprmpr/util/scoped_cleanup.h"

#include <gtest/gtest.h>

namespace mprmpr {

TEST(ScopedCleanup, TestCleanup) {
  int var = 0;
  {
    auto saved = var;
    auto cleanup = MakeScopedCleanup([&] () { var = saved; });
    var = 42;
  }
  ASSERT_EQ(0, var);
}

TEST(ScopedCleanup, TestCancelCleanup) {
  int var = 0;
  {
    auto saved = var;
    auto cleanup = MakeScopedCleanup([&] () { var = saved; });
    var = 42;
    cleanup.cancel();
  }
  ASSERT_EQ(42, var);
}

} // namespace mprmpr

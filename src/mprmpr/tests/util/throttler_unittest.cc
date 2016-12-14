#include "mprmpr/util/throttler.h"

#include <algorithm>
#include <cmath>
#include <cstring>

#include "mprmpr/util/test_util.h"

namespace mprmpr {

class ThrottlerTest : public AntTest {
};

TEST_F(ThrottlerTest, TestOpThrottle) {
  // Check operation rate throttling
  MonoTime now = MonoTime::Now();
  Throttler t0(now, 1000, 1000*1000, 1);
  // Fill up bucket
  now += MonoDelta::FromMilliseconds(2000);
  // Check throttle behavior for 1 second.
  for (int p = 0; p < 10; p++) {
    for (int i = 0; i < 100; i++) {
      ASSERT_TRUE(t0.Take(now, 1, 1));
    }
    ASSERT_FALSE(t0.Take(now, 1, 1));
    now += MonoDelta::FromMilliseconds(100);
  }
}

TEST_F(ThrottlerTest, TestIOThrottle) {
  // Check operation rate throttling
  MonoTime now = MonoTime::Now();
  Throttler t0(now, 50000, 1000*1000, 1);
  // Fill up bucket
  now += MonoDelta::FromMilliseconds(2000);
  // Check throttle behavior for 1 second.
  for (int p = 0; p < 10; p++) {
    for (int i = 0; i < 100; i++) {
      ASSERT_TRUE(t0.Take(now, 1, 1000));
    }
    ASSERT_FALSE(t0.Take(now, 1, 1000));
    now += MonoDelta::FromMilliseconds(100);
  }
}

TEST_F(ThrottlerTest, TestBurst) {
  // Check IO rate throttling
  MonoTime now = MonoTime::Now();
  Throttler t0(now, 2000, 1000*1000, 5);
  // Fill up bucket
  now += MonoDelta::FromMilliseconds(2000);
  for (int i = 0; i < 100; i++) {
    now += MonoDelta::FromMilliseconds(1);
    ASSERT_TRUE(t0.Take(now, 1, 5000));
  }
  ASSERT_TRUE(t0.Take(now, 1, 100000));
  ASSERT_FALSE(t0.Take(now, 1, 1));
}

} // namespace mprmpr

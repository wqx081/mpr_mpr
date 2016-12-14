#include <string>

#include <gtest/gtest.h>

#include "mprmpr/base/macros.h"
#include "mprmpr/util/errno.h"

using std::string;

namespace mprmpr {

TEST(OsUtilTest, TestErrnoToString) {
  int err = ENOENT;

  // Non-truncated result.
  ASSERT_EQ("No such file or directory", ErrnoToString(err));

  // Truncated because of a short buffer.
  char buf[2];
  ErrnoToCString(err, buf, arraysize(buf));
  ASSERT_EQ("N", string(buf));

  // Unknown error.
  string expected = "Unknown error";
  ASSERT_EQ(ErrnoToString(-1).compare(0, expected.length(), expected), 0);

  // Unknown error (truncated).
  ErrnoToCString(-1, buf, arraysize(buf));
  ASSERT_EQ("U", string(buf));
}

} // namespace mprmpr

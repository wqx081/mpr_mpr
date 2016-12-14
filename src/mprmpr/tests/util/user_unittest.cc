#include <string>

#include <gtest/gtest.h>

#include "mprmpr/util/status.h"
#include "mprmpr/util/test_util.h"
#include "mprmpr/util/user.h"

namespace mprmpr {

using std::string;

class TestUser : public AntTest {
};

// Validate that the current username is non-empty.
TEST_F(TestUser, TestNonEmpty) {
  string username;
  ASSERT_TRUE(username.empty());
  ASSERT_OK(GetLoggedInUser(&username));
  ASSERT_FALSE(username.empty());
  LOG(INFO) << "Name of the current user is: " << username;
}

} // namespace mprmpr

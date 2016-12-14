#include "mprmpr/util/env_util.h"

#include <gflags/gflags.h>
#include <memory>
#include <sys/statvfs.h>

#include "mprmpr/util/test_util.h"

DECLARE_int64(disk_reserved_bytes);
DECLARE_int64(disk_reserved_bytes_free_for_testing);

namespace mprmpr {

using std::string;
using std::unique_ptr;

class EnvUtilTest: public AntTest {
};

TEST_F(EnvUtilTest, TestDiskSpaceCheck) {
  Env* env = Env::Default();

  const int64_t kRequestedBytes = 0;
  int64_t reserved_bytes = 0;
  ASSERT_OK(env_util::VerifySufficientDiskSpace(env, test_dir_, kRequestedBytes, reserved_bytes));

  // Make it seem as if the disk is full and specify that we should have
  // reserved 200 bytes. Even asking for 0 bytes should return an error
  // indicating we are out of space.
  FLAGS_disk_reserved_bytes_free_for_testing = 0;
  reserved_bytes = 200;
  Status s = env_util::VerifySufficientDiskSpace(env, test_dir_, kRequestedBytes, reserved_bytes);
  ASSERT_TRUE(s.IsIOError());
  ASSERT_EQ(ENOSPC, s.posix_code());
  ASSERT_STR_CONTAINS(s.ToString(), "Insufficient disk space");
}

} // namespace mprmpr

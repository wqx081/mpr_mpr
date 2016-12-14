#include "mprmpr/util/user.h"

#include <sys/types.h>
#include <errno.h>
#include <pwd.h>
#include <unistd.h>

#include <string>

#include <glog/logging.h>

#include "mprmpr/base/gscoped_ptr.h"
#include "mprmpr/util/errno.h"
#include "mprmpr/util/status.h"

using std::string;

namespace mprmpr {

Status GetLoggedInUser(string* user_name) {
  DCHECK(user_name != nullptr);

  struct passwd pwd;
  struct passwd *result;

  size_t bufsize = sysconf(_SC_GETPW_R_SIZE_MAX);
  if (bufsize == -1) {  // Value was indeterminate.
    bufsize = 16384;    // Should be more than enough, per the man page.
  }

  gscoped_ptr<char[], base::FreeDeleter> buf(static_cast<char *>(malloc(bufsize)));
  if (buf.get() == nullptr) {
    return Status::RuntimeError("Malloc failed", ErrnoToString(errno), errno);
  }

  int ret = getpwuid_r(getuid(), &pwd, buf.get(), bufsize, &result);
  if (result == nullptr) {
    if (ret == 0) {
      return Status::NotFound("Current logged-in user not found! This is an unexpected error.");
    } else {
      // Errno in ret
      return Status::RuntimeError("Error calling getpwuid_r()", ErrnoToString(ret), ret);
    }
  }

  *user_name = pwd.pw_name;

  return Status::OK();
}

} // namespace mprmpr

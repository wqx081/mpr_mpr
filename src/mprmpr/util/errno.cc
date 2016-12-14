#include "mprmpr/util/errno.h"

#include <errno.h>
#include <string.h>

#include <glog/logging.h>

namespace mprmpr {

void ErrnoToCString(int err, char *buf, size_t buf_len) {
  CHECK_GT(buf_len, 0);
  // Using GLIBC version
  char* ret = strerror_r(err, buf, buf_len);
  if (ret != buf) {
    strncpy(buf, ret, buf_len);
    buf[buf_len - 1] = '\0';
  }
}
} // namespace mprmpr

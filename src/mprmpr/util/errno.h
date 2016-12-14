#ifndef KUDU_ERRNO_H
#define KUDU_ERRNO_H

#include <string>

namespace mprmpr {

void ErrnoToCString(int err, char *buf, size_t buf_len);

// Return a string representing an errno.
inline static std::string ErrnoToString(int err) {
  char buf[512];
  ErrnoToCString(err, buf, sizeof(buf));
  return std::string(buf);
}

} // namespace mprmpr

#endif

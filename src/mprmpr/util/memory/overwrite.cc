#include "mprmpr/util/memory/overwrite.h"
#include "mprmpr/base/strings/stringpiece.h"

#include <string.h>
#include <glog/logging.h>

namespace mprmpr {

void OverwriteWithPattern(char* p, size_t len, StringPiece pattern) {
  size_t pat_len = pattern.size();
  CHECK_LT(0, pat_len);
  size_t rem = len;
  const char *pat_ptr = pattern.data();

  while (rem >= pat_len) {
    memcpy(p, pat_ptr, pat_len);
    p += pat_len;
    rem -= pat_len;
  }

  while (rem-- > 0) {
    *p++ = *pat_ptr++;
  }
}

} // namespace mprmpr

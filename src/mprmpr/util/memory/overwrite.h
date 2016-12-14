#ifndef KUDU_MEMORY_OVERWRITE_H
#define KUDU_MEMORY_OVERWRITE_H

#include "mprmpr/base/strings/stringpiece.h"

namespace mprmpr {

// Overwrite 'p' with enough repetitions of 'pattern' to fill 'len'
// bytes. This is optimized at -O3 even in debug builds, so is
// reasonably efficient to use.
void OverwriteWithPattern(char* p, size_t len, StringPiece pattern);

} // namespace mprmpr
#endif /* KUDU_MEMORY_OVERWRITE_H */


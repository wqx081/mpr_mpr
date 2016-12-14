#include "mprmpr/util/malloc.h"

#include <malloc.h>

namespace mprmpr {

int64_t ant_malloc_usable_size(const void* obj) {
  return malloc_usable_size(const_cast<void*>(obj));
}

} // namespace mprmpr

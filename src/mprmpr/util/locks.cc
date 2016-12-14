#include "mprmpr/util/locks.h"
#include "mprmpr/util/malloc.h"

namespace mprmpr {

size_t percpu_rwlock::memory_footprint_excluding_this() const {
  return n_cpus_ * sizeof(padded_lock);
}

size_t percpu_rwlock::memory_footprint_including_this() const {
  return ant_malloc_usable_size(this) + memory_footprint_excluding_this();
}

} // namespace mprmpr

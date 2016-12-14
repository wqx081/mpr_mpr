#ifndef ANT_UTIL_YIELD_H_
#define ANT_UTIL_YIELD_H_

#include <sched.h>
#include <time.h>

namespace mprmpr {

inline void yield(unsigned k) {
  if (k < 4) {
    // Nothing to do.
  } else if (k < 16) {
    __asm__ __volatile__( "rep; nop" : : : "memory" );
  } else if (k < 32 || k & 1) {
    sched_yield();
  } else {
    // g++ -Wextra warns on {} or {0}
    struct timespec t = {0, 0};
    t.tv_sec = 0;
    t.tv_nsec = 1000;
   
    nanosleep(&t, 0);
  }
}

} // namespace mprmpr
#endif // ANT_UTIL_YIELD_H_

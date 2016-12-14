#ifndef ANT_UTIL_THREAD_STATS_H_
#define ANT_UTIL_THREAD_STATS_H_
#include <string>

#include "mprmpr/util/status.h"

namespace mprmpr {

struct ThreadStats {
  int64_t user_ns;
  int64_t kernel_ns;
  int64_t iowait_ns;

  ThreadStats() : user_ns(0), kernel_ns(0), iowait_ns(0) {}
};

Status ParseStat(const std::string& buffer, std::string* name, ThreadStats* stats);
Status GetThreadStats(int64_t tid, ThreadStats* stats);
void DisableCoreDumps();

} // namespace mprmpr

#endif // ANT_UTIL_THREAD_STATS_H_

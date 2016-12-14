#include "mprmpr/util/thread_stats.h"

#include <fcntl.h>
#include <fstream>
#include <sstream>
#include <string>
#include <sys/resource.h>
#include <vector>
#include <unistd.h>

#include "mprmpr/base/strings/numbers.h"
#include "mprmpr/base/strings/split.h"
#include "mprmpr/base/strings/substitute.h"
#include "mprmpr/util/errno.h"

using std::ifstream;
using std::istreambuf_iterator;
using std::ostringstream;
using strings::Split;
using strings::Substitute;

namespace mprmpr {

// Ensure that Impala compiles on earlier kernels. If the target kernel does not support
// _SC_CLK_TCK, sysconf(_SC_CLK_TCK) will return -1.
#ifndef _SC_CLK_TCK
#define _SC_CLK_TCK 2
#endif

static const int64_t TICKS_PER_SEC = sysconf(_SC_CLK_TCK);

// Offsets into the ../stat file array of per-thread statistics.
//
// They are themselves offset by two because the pid and comm fields of the
// file are parsed separately.
static const int64_t USER_TICKS = 13 - 2;
static const int64_t KERNEL_TICKS = 14 - 2;
static const int64_t IO_WAIT = 41 - 2;

// Largest offset we are interested in, to check we get a well formed stat file.
static const int64_t MAX_OFFSET = IO_WAIT;

Status ParseStat(const std::string& buffer, std::string* name, ThreadStats* stats) {
  DCHECK(stats != nullptr);

  // The thread name should be the only field with parentheses. But the name
  // itself may contain parentheses.
  size_t open_paren = buffer.find('(');
  size_t close_paren = buffer.rfind(')');
  if (open_paren == string::npos  ||      // '(' must exist
      close_paren == string::npos ||      // ')' must exist
      open_paren >= close_paren   ||      // '(' must come before ')'
      close_paren + 2 == buffer.size()) { // there must be at least two chars after ')'
    return Status::IOError("Unrecognised /proc format");
  }
  string extracted_name = buffer.substr(open_paren + 1, close_paren - (open_paren + 1));
  string rest = buffer.substr(close_paren + 2);
  vector<string> splits = Split(rest, " ", strings::SkipEmpty());
  if (splits.size() < MAX_OFFSET) {
    return Status::IOError("Unrecognised /proc format");
  }

  int64 tmp;
  if (safe_strto64(splits[USER_TICKS], &tmp)) {
    stats->user_ns = tmp * (1e9 / TICKS_PER_SEC);
  }
  if (safe_strto64(splits[KERNEL_TICKS], &tmp)) {
    stats->kernel_ns = tmp * (1e9 / TICKS_PER_SEC);
  }
  if (safe_strto64(splits[IO_WAIT], &tmp)) {
    stats->iowait_ns = tmp * (1e9 / TICKS_PER_SEC);
  }
  if (name != nullptr) {
    *name = extracted_name;
  }
  return Status::OK();

}

Status GetThreadStats(int64_t tid, ThreadStats* stats) {
  DCHECK(stats != nullptr);
  if (TICKS_PER_SEC <= 0) {
    return Status::NotSupported("ThreadStats not supported");
  }

  ostringstream proc_path;
  proc_path << "/proc/self/task/" << tid << "/stat";
  ifstream proc_file(proc_path.str().c_str());
  if (!proc_file.is_open()) {
    return Status::IOError("Could not open ifstream");
  }

  string buffer((istreambuf_iterator<char>(proc_file)),
      istreambuf_iterator<char>());

  return ParseStat(buffer, nullptr, stats); // don't want the name
}

void DisableCoreDumps() {
  struct rlimit lim;
  PCHECK(getrlimit(RLIMIT_CORE, &lim) == 0);
  lim.rlim_cur = 0;
  PCHECK(setrlimit(RLIMIT_CORE, &lim) == 0);

  // Set coredump_filter to not dump any parts of the address space.
  // Although the above disables core dumps to files, if core_pattern
  // is set to a pipe rather than a file, it's not sufficient. Setting
  // this pattern results in piping a very minimal dump into the core
  // processor (eg abrtd), thus speeding up the crash.
  int f = open("/proc/self/coredump_filter", O_WRONLY);
  if (f >= 0) {
    write(f, "00000000", 8);
    close(f);
  }
}


} // namespace mprmpr

#ifndef KUDU_UTIL_DEBUG_UTIL_H
#define KUDU_UTIL_DEBUG_UTIL_H

#include <sys/types.h>

#include <string>
#include <vector>

#include "mprmpr/base/strings/fastmem.h"
#include "mprmpr/util/status.h"

namespace mprmpr {

bool IsCoverageBuild();
void TryFlushCoverage();
Status ListThreads(std::vector<pid_t>* tids);
Status SetStackTraceSignal(int signum);
std::string DumpThreadStack(int64_t tid);
std::string GetStackTrace();
std::string GetStackTraceHex();
std::string GetLogFormatStackTraceHex();
void HexStackTraceToString(char* buf, size_t size);

// Efficient class for collecting and later stringifying a stack trace.
//
// Requires external synchronization.
class StackTrace {
 public:
  StackTrace()
    : num_frames_(0) {
  }

  void Reset() {
    num_frames_ = 0;
  }

  void CopyFrom(const StackTrace& s) {
    memcpy(this, &s, sizeof(s));
  }

  bool Equals(const StackTrace& s) {
    return s.num_frames_ == num_frames_ &&
      strings::memeq(frames_, s.frames_,
                     num_frames_ * sizeof(frames_[0]));
  }

  // Collect and store the current stack trace. Skips the top 'skip_frames' frames
  // from the stack. For example, a value of '1' will skip the 'Collect()' function
  // call itself.
  //
  // This function is technically not async-safe. However, according to
  // http://lists.nongnu.org/archive/html/libunwind-devel/2011-08/msg00054.html it is "largely
  // async safe" and it would only deadlock in the case that you call it while a dynamic library
  // load is in progress. We assume that dynamic library loads would almost always be completed
  // very early in the application lifecycle, so for now, this is considered "async safe" until
  // it proves to be a problem.
  void Collect(int skip_frames = 1);


  enum Flags {
    // Do not fix up the addresses on the stack to try to point to the 'call'
    // instructions instead of the return address. This is necessary when dumping
    // addresses to be interpreted by 'pprof', which does this fix-up itself.
    NO_FIX_CALLER_ADDRESSES = 1
  };

  // Stringify the trace into the given buffer.
  // The resulting output is hex addresses suitable for passing into 'addr2line'
  // later.
  void StringifyToHex(char* buf, size_t size, int flags = 0) const;

  // Same as above, but returning a std::string.
  // This is not async-safe.
  std::string ToHexString(int flags = 0) const;

  // Return a string with a symbolized backtrace in a format suitable for
  // printing to a log file.
  // This is not async-safe.
  std::string Symbolize() const;

  // Return a string with a hex-only backtrace in the format typically used in
  // log files. Similar to the format given by Symbolize(), but symbols are not
  // resolved (only the hex addresses are given).
  std::string ToLogFormatHexString() const;

  uint64_t HashCode() const;

 private:
  enum {
    // The maximum number of stack frames to collect.
    kMaxFrames = 16,

    // The max number of characters any frame requires in string form.
    kHexEntryLength = 16
  };

  int num_frames_;
  void* frames_[kMaxFrames];
};

} // namespace mprmpr

#endif

#ifndef _SYSINFO_H_
#define _SYSINFO_H_

#include <stdint.h>

namespace base {

// Return the number of online CPUs. This is computed and cached the first time this or
// NumCPUs() is called, so does not reflect any CPUs enabled or disabled at a later
// point in time.
//
// Note that, if not all CPUs are online, this may return a value lower than the maximum
// value of sched_getcpu().
extern int NumCPUs();

// Return the maximum CPU index that may be returned by sched_getcpu(). For example, on
// an 8-core machine, this will return '7' even if some of the CPUs have been disabled.
extern int MaxCPUIndex();

void SleepForNanoseconds(int64_t nanoseconds);
void SleepForMilliseconds(int64_t milliseconds);

// processor cycles per second of each processor.  Thread-safe.
extern double CyclesPerSecond(void);

} // namespace base
#endif   /* #ifndef _SYSINFO_H_ */

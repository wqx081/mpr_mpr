#ifndef KUDU_UTIL_SEMAPHORE_H
#define KUDU_UTIL_SEMAPHORE_H

#include <semaphore.h>

#include "mprmpr/base/macros.h"
#include "mprmpr/base/port.h"
#include "mprmpr/util/monotime.h"

namespace mprmpr {

// Wrapper for POSIX semaphores.
class Semaphore {
 public:
  // Initialize the semaphore with the specified capacity.
  explicit Semaphore(int capacity);
  ~Semaphore();

  // Acquire the semaphore.
  void Acquire();

  // Acquire the semaphore within the given timeout. Returns true if successful.
  bool TimedAcquire(const MonoDelta& timeout);

  // Try to acquire the semaphore immediately. Returns false if unsuccessful.
  bool TryAcquire();

  // Release the semaphore.
  void Release();

  // Get the current value of the semaphore.
  int GetValue();

  // Boost-compatible wrappers.
  void lock() { Acquire(); }
  void unlock() { Release(); }
  bool try_lock() { return TryAcquire(); }

 private:
  void Fatal(const char* action) ATTRIBUTE_NORETURN;

  sem_t sem_;
  DISALLOW_COPY_AND_ASSIGN(Semaphore);
};

} // namespace mprmpr
#endif /* KUDU_UTIL_SEMAPHORE_H */

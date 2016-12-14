#pragma once

#include <pthread.h>
#include <unordered_set>

#include "mprmpr/base/macros.h"
#include "mprmpr/util/locks.h"

namespace mprmpr {

// Read/write mutex. Implemented as a thin wrapper around pthread_rwlock_t.
//
// Although pthread_rwlock_t allows recursive acquisition, this wrapper does
// not, and will crash in debug mode if recursive acquisition is detected.
class RWMutex {
 public:

  // Possible fairness policies for the RWMutex.
  enum class Priority {
    // The lock will prioritize readers at the expense of writers.
    PREFER_READING,

    // The lock will prioritize writers at the expense of readers.
    //
    // Care should be taken when using this fairness policy, as it can lead to
    // unexpected deadlocks (e.g. a writer waiting on the lock will prevent
    // additional readers from acquiring it).
    PREFER_WRITING,
  };

  // Create an RWMutex that prioritizes readers.
  RWMutex();

  // Create an RWMutex with customized priority. This is a best effort; the
  // underlying platform may not support custom priorities.
  explicit RWMutex(Priority prio);

  ~RWMutex();

  void ReadLock();
  void ReadUnlock();
  bool TryReadLock();

  void WriteLock();
  void WriteUnlock();
  bool TryWriteLock();

#ifndef NDEBUG
  void AssertAcquiredForReading() const;
  void AssertAcquiredForWriting() const;
#else
  void AssertAcquiredForReading() const {}
  void AssertAcquiredForWriting() const {}
#endif

  // Aliases for use with std::lock_guard and mprmpr::shared_lock.
  void lock() { WriteLock(); }
  void unlock() { WriteUnlock(); }
  bool try_lock() { return TryWriteLock(); }
  void lock_shared() { ReadLock(); }
  void unlock_shared() { ReadUnlock(); }
  bool try_lock_shared() { return TryReadLock(); }

 private:
  void Init(Priority prio);

  enum class LockState {
    NEITHER,
    READER,
    WRITER,
  };
#ifndef NDEBUG
  void CheckLockState(LockState state) const;
  void MarkForReading();
  void MarkForWriting();
  void UnmarkForReading();
  void UnmarkForWriting();
#else
  void CheckLockState(LockState state) const {}
  void MarkForReading() {}
  void MarkForWriting() {}
  void UnmarkForReading() {}
  void UnmarkForWriting() {}
#endif

  pthread_rwlock_t native_handle_;

#ifndef NDEBUG
  // Protects reader_tids_ and writer_tid_.
  mutable simple_spinlock tid_lock_;

  // Tracks all current readers by tid.
  std::unordered_set<pid_t> reader_tids_;

  // Tracks the current writer (if one exists) by tid.
  pid_t writer_tid_;
#endif

  DISALLOW_COPY_AND_ASSIGN(RWMutex);
};

} // namespace mprmpr

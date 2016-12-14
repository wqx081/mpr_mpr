#ifndef KUDU_UTIL_ATOMIC_H
#define KUDU_UTIL_ATOMIC_H

#include <algorithm>
#include <type_traits>

#include "mprmpr/base/atomicops.h"
#include "mprmpr/base/macros.h"
#include "mprmpr/base/port.h"

namespace mprmpr {

enum MemoryOrder {
  kMemOrderNoBarrier = 0,
  kMemOrderAcquire = 1,
  kMemOrderRelease = 2,
  kMemOrderBarrier = 3,
};

template<typename T>
class AtomicInt {
 public:
  explicit AtomicInt(T initial_value);

  T Load(MemoryOrder mem_order = kMemOrderNoBarrier) const;
  void Store(T new_value, MemoryOrder mem_order = kMemOrderNoBarrier);
  bool CompareAndSet(T expected_val, T new_value, MemoryOrder mem_order = kMemOrderNoBarrier);
  T CompareAndSwap(T expected_val, T new_value, MemoryOrder mem_order = kMemOrderNoBarrier);
  void StoreMax(T new_value, MemoryOrder mem_order = kMemOrderNoBarrier);
  void StoreMin(T new_value, MemoryOrder mem_order = kMemOrderNoBarrier);
  T Increment(MemoryOrder mem_order = kMemOrderNoBarrier);
  T IncrementBy(T delta, MemoryOrder mem_order = kMemOrderNoBarrier);
  T Exchange(T new_value, MemoryOrder mem_order = kMemOrderNoBarrier);

 private:
  static void FatalMemOrderNotSupported(const char* caller,
                                        const char* requested = "kMemOrderBarrier",
                                        const char* supported =
                                        "kMemNorderNoBarrier, kMemOrderAcquire, kMemOrderRelease");

  typedef typename std::make_signed<T>::type SignedT;
  SignedT value_;

  DISALLOW_COPY_AND_ASSIGN(AtomicInt);
};

class AtomicBool {
 public:
  explicit AtomicBool(bool value);

  bool Load(MemoryOrder m = kMemOrderNoBarrier) const {
    return underlying_.Load(m);
  }
  void Store(bool n, MemoryOrder m = kMemOrderNoBarrier) {
    underlying_.Store(static_cast<int32_t>(n), m);
  }
  bool CompareAndSet(bool e, bool n, MemoryOrder m = kMemOrderNoBarrier) {
    return underlying_.CompareAndSet(static_cast<int32_t>(e), static_cast<int32_t>(n), m);
  }
  bool CompareAndSwap(bool e, bool n, MemoryOrder m = kMemOrderNoBarrier) {
    return underlying_.CompareAndSwap(static_cast<int32_t>(e), static_cast<int32_t>(n), m);
  }
  bool Exchange(bool n, MemoryOrder m = kMemOrderNoBarrier) {
    return underlying_.Exchange(static_cast<int32_t>(n), m);
  }
 private:
  AtomicInt<int32_t> underlying_;

  DISALLOW_COPY_AND_ASSIGN(AtomicBool);
};

template<typename T>
inline T AtomicInt<T>::Load(MemoryOrder mem_order) const {
  switch (mem_order) {
    case kMemOrderNoBarrier: {
      return base::subtle::NoBarrier_Load(&value_);
    }
    case kMemOrderBarrier: {
      FatalMemOrderNotSupported("Load");
      break;
    }
    case kMemOrderAcquire: {
      return base::subtle::Acquire_Load(&value_);
    }
    case kMemOrderRelease: {
      return base::subtle::Release_Load(&value_);
    }
  }
  abort(); // Unnecessary, but avoids gcc complaining.
}

template<typename T>
inline void AtomicInt<T>::Store(T new_value, MemoryOrder mem_order) {
  switch (mem_order) {
    case kMemOrderNoBarrier: {
      base::subtle::NoBarrier_Store(&value_, new_value);
      break;
    }
    case kMemOrderBarrier: {
      FatalMemOrderNotSupported("Store");
      break;
    }
    case kMemOrderAcquire: {
      base::subtle::Acquire_Store(&value_, new_value);
      break;
    }
    case kMemOrderRelease: {
      base::subtle::Release_Store(&value_, new_value);
      break;
    }
  }
}

template<typename T>
inline bool AtomicInt<T>::CompareAndSet(T expected_val, T new_val, MemoryOrder mem_order) {
  return CompareAndSwap(expected_val, new_val, mem_order) == expected_val;
}

template<typename T>
inline T AtomicInt<T>::CompareAndSwap(T expected_val, T new_val, MemoryOrder mem_order) {
  switch (mem_order) {
    case kMemOrderNoBarrier: {
      return base::subtle::NoBarrier_CompareAndSwap(
          &value_, expected_val, new_val);
    }
    case kMemOrderBarrier: {
      FatalMemOrderNotSupported("CompareAndSwap/CompareAndSet");
      break;
    }
    case kMemOrderAcquire: {
      return base::subtle::Acquire_CompareAndSwap(
          &value_, expected_val, new_val);
    }
    case kMemOrderRelease: {
      return base::subtle::Release_CompareAndSwap(
          &value_, expected_val, new_val);
    }
  }
  abort();
}


template<typename T>
inline T AtomicInt<T>::Increment(MemoryOrder mem_order) {
  return IncrementBy(1, mem_order);
}

template<typename T>
inline T AtomicInt<T>::IncrementBy(T delta, MemoryOrder mem_order) {
  switch (mem_order) {
    case kMemOrderNoBarrier: {
      return base::subtle::NoBarrier_AtomicIncrement(&value_, delta);
    }
    case kMemOrderBarrier: {
      return base::subtle::Barrier_AtomicIncrement(&value_, delta);
    }
    case kMemOrderAcquire: {
      FatalMemOrderNotSupported("Increment/IncrementBy",
                                "kMemOrderAcquire",
                                "kMemOrderNoBarrier and kMemOrderBarrier");
      break;
    }
    case kMemOrderRelease: {
      FatalMemOrderNotSupported("Increment/Incrementby",
                                "kMemOrderAcquire",
                                "kMemOrderNoBarrier and kMemOrderBarrier");
      break;
    }
  }
  abort();
}

template<typename T>
inline T AtomicInt<T>::Exchange(T new_value, MemoryOrder mem_order) {
  switch (mem_order) {
    case kMemOrderNoBarrier: {
      return base::subtle::NoBarrier_AtomicExchange(&value_, new_value);
    }
    case kMemOrderBarrier: {
      FatalMemOrderNotSupported("Exchange");
      break;
    }
    case kMemOrderAcquire: {
      //return base::subtle::Acquire_AtomicExchange(&value_, new_value);
      //TODO(wqx):
      return base::subtle::NoBarrier_AtomicExchange(&value_, new_value);
    }
    case kMemOrderRelease: {
      return base::subtle::NoBarrier_AtomicExchange(&value_, new_value);
    }
  }
  abort();
}

template<typename T>
inline void AtomicInt<T>::StoreMax(T new_value, MemoryOrder mem_order) {
  T old_value = Load(mem_order);
  while (true) {
    T max_value = std::max(old_value, new_value);
    T prev_value = CompareAndSwap(old_value, max_value, mem_order);
    if (PREDICT_TRUE(old_value == prev_value)) {
      break;
    }
    old_value = prev_value;
  }
}

template<typename T>
inline void AtomicInt<T>::StoreMin(T new_value, MemoryOrder mem_order) {
  T old_value = Load(mem_order);
  while (true) {
    T min_value = std::min(old_value, new_value);
    T prev_value = CompareAndSwap(old_value, min_value, mem_order);
    if (PREDICT_TRUE(old_value == prev_value)) {
      break;
    }
    old_value = prev_value;
  }
}

} // namespace mprmpr
#endif /* KUDU_UTIL_ATOMIC_H */

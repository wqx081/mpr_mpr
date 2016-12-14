#ifndef KUDU_BOOST_MUTEX_UTILS_H
#define KUDU_BOOST_MUTEX_UTILS_H


// Similar to std::lock_guard except that it takes
// a lock pointer, and checks against nullptr. If the
// pointer is NULL, does nothing. Otherwise guards
// with the lock.
template<class LockType>
class lock_guard_maybe {
 public:
  explicit lock_guard_maybe(LockType *l) :
    lock_(l) {
    if (l != nullptr) {
      l->lock();
    }
  }

  ~lock_guard_maybe() {
    if (lock_ != nullptr) {
      lock_->unlock();
    }
  }

 private:
  LockType *lock_;
};

#endif

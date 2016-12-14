#ifndef ANT_UTIL_OPTIONAL_H_
#define ANT_UTIL_OPTIONAL_H_

#include <algorithm>
#include <memory>
#include <utility>
#include <glog/logging.h>

namespace mprmpr {

namespace optional_internal {

template<typename T>
inline T* FunctionThatDoesNothing(T* x) { return x; }

} // namespace optional_internal


template<typename T>
class Optional final {
 public:
  Optional() : has_value_(false), empty_('\0') {}

  explicit Optional(const T& value) : has_value_(true) {
    new (&value_) T(value);
  }
  explicit Optional(T&& value) : has_value_(true) {
    new (&value_) T(std::move(value));
  }

  Optional(const Optional& m) : has_value_(m.has_value_) {
    if (has_value_) {
      new (&value_) T(m.value_);
    }
  }

  Optional(Optional&& m) : has_value_(m.has_value_) {
    if (has_value_) {
      new (&value_) T(std::move(m.value_));
    }
  }

  ~Optional() {
    if (has_value_) {
      value_.~T();
    }
  }

  Optional& operator=(const Optional& m) {
    if (m.has_value_) {
      if (has_value_) {
        value_ = m.value_;  // T's copy assignment.
      } else {
        //UnpoisonValue();
        new (&value_) T(m.value_);  // T's copy constructor.
        has_value_ = true;
      }
    } else {
      reset();
    }
    return *this;
  }

  Optional& operator=(Optional&& m) {
    if (m.has_value_) {
      if (has_value_) {
        value_ = std::move(m.value_);  // T's move assignment.
      } else {
//        UnpoisonValue();
        new (&value_) T(std::move(m.value_));  // T's move constructor.
        has_value_ = true;
      }
    } else {
      reset();
    }
    return *this;
  }

  friend void swap(Optional& m1, Optional& m2) {
    if (m1.has_value_) {
      if (m2.has_value_) {
        using std::swap;
        swap(m1.value_, m2.value_);
      } else {
//        m2.UnpoisonValue();
        new (&m2.value_) T(std::move(m1.value_));
        m1.value_.~T();  // Destroy the moved-from value.
        m1.has_value_ = false;
        m2.has_value_ = true;
//        m1.PoisonValue();
      } 
    } else if (m2.has_value_) {
//      m1.UnpoisonValue();
      new (&m1.value_) T(std::move(m2.value_));
      m2.value_.~T();  // Destroy the moved-from value.
      m1.has_value_ = true;
      m2.has_value_ = false;
//      m2.PoisonValue();
    } 
  } 

  void reset() {
    if (!has_value_)
      return;
    value_.~T();
    has_value_ = false;
//    PoisonValue();
  } 

  template<typename... Args>
  void emplace(Args&&... args) {
    if (has_value_)
      value_.~T();
    new (&value_) T(std::forward<Args>(args)...);
    has_value_ = true;
  }

  explicit operator bool() const { return has_value_; }

  const T* operator->() const {
    DCHECK(has_value_);
    return &value_;
  }

  T* operator->() {
    DCHECK(has_value_);
    return &value_;
  }

  const T& operator*() const {
    DCHECK(has_value_);
    return value_;
  }

  T& operator*() {
    DCHECK(has_value_);
    return value_;
  }

  const T& value_or(const T& default_val) const {
    return has_value_ ? *optional_internal::FunctionThatDoesNothing(&value_)
                      : default_val;
  }

  friend bool operator==(const Optional& m1, const Optional& m2) {
    return m1.has_value_ && m2.has_value_ ? m1.value_ == m2.value_
                                          : m1.has_value_ == m2.has_value_;
  }
  friend bool operator==(const Optional& opt, const T& value) {
    return opt.has_value_ && opt.value_ == value;
  }
  friend bool operator==(const T& value, const Optional& opt) {
    return opt.has_value_ && value == opt.value_;
  }

  friend bool operator!=(const Optional& m1, const Optional& m2) {
    return m1.has_value_ && m2.has_value_ ? m1.value_ != m2.value_
                                          : m1.has_value_ != m2.has_value_;
  }
  friend bool operator!=(const Optional& opt, const T& value) {
    return !opt.has_value_ || opt.value_ != value;
  }
  friend bool operator!=(const T& value, const Optional& opt) {
    return !opt.has_value_ || value != opt.value_;
  }

 private:
  bool has_value_;
  union {
    char empty_;
    T value_;
  };
};

} // namespace mprmpr
#endif // ANT_UTIL_OPTIONAL_H_

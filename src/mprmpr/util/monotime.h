#ifndef KUDU_UTIL_MONOTIME_H
#define KUDU_UTIL_MONOTIME_H

#include <stdint.h>
#include <string>

#include <gtest/gtest_prod.h>


struct timeval;
struct timespec;

namespace mprmpr {
class MonoTime;

class  MonoDelta {
 public:
  static MonoDelta FromSeconds(double seconds);
  static MonoDelta FromMilliseconds(int64_t ms);
  static MonoDelta FromMicroseconds(int64_t us);
  static MonoDelta FromNanoseconds(int64_t ns);

  MonoDelta();

  bool Initialized() const;

  bool LessThan(const MonoDelta &rhs) const;

  bool MoreThan(const MonoDelta &rhs) const;

  bool Equals(const MonoDelta &rhs) const;

  std::string ToString() const;

  double ToSeconds() const;
  int64_t ToMilliseconds() const;
  int64_t ToMicroseconds() const;
  int64_t ToNanoseconds() const;

  void ToTimeVal(struct timeval *tv) const;

  void ToTimeSpec(struct timespec *ts) const;

  static void NanosToTimeSpec(int64_t nanos, struct timespec* ts);

 private:
  static const int64_t kUninitialized;

  friend class MonoTime;
  FRIEND_TEST(TestMonoTime, TestDeltaConversions);

  explicit MonoDelta(int64_t delta);
  int64_t nano_delta_;
};

class  MonoTime {
 public:
  static const int64_t kNanosecondsPerSecond = 1000000000L;
  static const int64_t kNanosecondsPerMillisecond = 1000000L;
  static const int64_t kNanosecondsPerMicrosecond = 1000L;

  static const int64_t kMicrosecondsPerSecond = 1000000L;

  static MonoTime Now();

  static MonoTime Max();

  static MonoTime Min();

  static const MonoTime& Earliest(const MonoTime& a, const MonoTime& b);

  MonoTime();

  bool Initialized() const;

  MonoDelta GetDeltaSince(const MonoTime &rhs) const;

  void AddDelta(const MonoDelta &delta);

  bool ComesBefore(const MonoTime &rhs) const;

  std::string ToString() const;

  bool Equals(const MonoTime& other) const;

  MonoTime& operator+=(const MonoDelta& delta);

  MonoTime& operator-=(const MonoDelta& delta);

 private:
  friend class MonoDelta;
  FRIEND_TEST(TestMonoTime, TestTimeSpec);
  FRIEND_TEST(TestMonoTime, TestDeltaConversions);

  explicit MonoTime(const struct timespec &ts);
  explicit MonoTime(int64_t nanos);
  double ToSeconds() const;
  int64_t nanos_;
};

/// Sleep for an interval specified by a MonoDelta instance.
///
/// This is preferred over sleep(3), usleep(3), and nanosleep(3).
/// It's less prone to mixups with units since it uses the MonoDelta for
/// interval specification.
/// Besides, it ignores signals/EINTR, so will reliably sleep at least for the
/// MonoDelta duration.
///
/// @param [in] delta
///   The time interval to sleep for.
void  SleepFor(const MonoDelta& delta);

/// @name Syntactic sugar: binary operators for MonoDelta.
///@{
///
/// @param [in] lhs
///   A time interval for comparison: the left-hand operand.
/// @param [in] rhs
///   A time interval for comparison: the right-hand operand.
/// @return @c true iff the time interval represented by @c lhs is equal
///   to the time interval represented by @c rhs.
bool  operator==(const MonoDelta &lhs, const MonoDelta &rhs);

/// @param [in] lhs
///   A time interval for comparison: the left-hand operand.
/// @param [in] rhs
///   A time interval for comparison: the right-hand operand.
/// @return @c true iff the time interval represented by @c lhs is not equal
///   to the time interval represented by @c rhs.
bool  operator!=(const MonoDelta &lhs, const MonoDelta &rhs);

/// @param [in] lhs
///   A time interval for comparison: the left-hand operand.
/// @param [in] rhs
///   A time interval for comparison: the right-hand operand.
/// @return @c true iff the time interval represented by @c lhs is shorter
///   than the time interval represented by @c rhs.
bool  operator<(const MonoDelta &lhs, const MonoDelta &rhs);

/// @param [in] lhs
///   A time interval for comparison: the left-hand operand.
/// @param [in] rhs
///   A time interval for comparison: the right-hand operand.
/// @return @c true iff the time interval represented by @c lhs is shorter
///   than or equal to the time interval represented by @c rhs.
bool  operator<=(const MonoDelta &lhs, const MonoDelta &rhs);

/// @param [in] lhs
///   A time interval for comparison: the left-hand operand.
/// @param [in] rhs
///   A time interval for comparison: the right-hand operand.
/// @return @c true iff the time interval represented by @c lhs is longer
///   than the time interval represented by @c rhs.
bool  operator>(const MonoDelta &lhs, const MonoDelta &rhs);

/// @param [in] lhs
///   A time interval for comparison: the left-hand operand.
/// @param [in] rhs
///   A time interval for comparison: the right-hand operand.
/// @return @c true iff the time interval represented by @c lhs is longer
///   than or equal to the time interval represented by @c rhs.
bool  operator>=(const MonoDelta &lhs, const MonoDelta &rhs);
///@}

/// @name Syntactic sugar: binary operators for MonoTime.
///@{
///
/// Check if the specified objects represent the same point in time.
///
/// This is a handy operator which is semantically equivalent to
/// MonoTime::Equals().
///
/// @param [in] lhs
///   The left-hand operand.
/// @param [in] rhs
///   The right-hand operand.
/// @return @c true iff the given objects represent the same point in time.
bool  operator==(const MonoTime& lhs, const MonoTime& rhs);

/// Check if the specified objects represent different points in time.
///
/// This is a handy operator which is semantically equivalent to the negation of
/// MonoTime::Equals().
///
/// @param [in] lhs
///   The left-hand operand.
/// @param [in] rhs
///   The right-hand operand.
/// @return @c true iff the given object represents a different point in time
///   than the specified one.
bool  operator!=(const MonoTime& lhs, const MonoTime& rhs);

/// @param [in] lhs
///   The left-hand operand.
/// @param [in] rhs
///   The right-hand operand.
/// @return @c true iff the @c lhs object represents an earlier point in time
///   than the @c rhs object.
bool  operator<(const MonoTime& lhs, const MonoTime& rhs);

/// @param [in] lhs
///   The left-hand operand.
/// @param [in] rhs
///   The right-hand operand.
/// @return @c true iff the @c lhs object represents an earlier than or
///   the same point in time as the @c rhs object.
bool  operator<=(const MonoTime& lhs, const MonoTime& rhs);

/// @param [in] lhs
///   The left-hand operand.
/// @param [in] rhs
///   The right-hand operand.
/// @return @c true iff the @c lhs object represents a later point in time
///   than the @c rhs object.
bool  operator>(const MonoTime& lhs, const MonoTime& rhs);

/// @param [in] lhs
///   The left-hand operand.
/// @param [in] rhs
///   The right-hand operand.
/// @return @c true iff the @c lhs object represents a later than or
///   the same point in time as the @c rhs object.
bool  operator>=(const MonoTime& lhs, const MonoTime& rhs);
///@}

/// @name Syntactic sugar: mixed binary operators for MonoTime/MonoDelta.
///@{
///
/// Add the specified time interval to the given point in time.
///
/// @param [in] t
///   A MonoTime object representing the given point in time.
/// @param [in] delta
///   A MonoDelta object representing the specified time interval.
/// @return A MonoTime object representing the resulting point in time.
MonoTime  operator+(const MonoTime& t, const MonoDelta& delta);

/// Subtract the specified time interval from the given point in time.
///
/// @param [in] t
///   A MonoTime object representing the given point in time.
/// @param [in] delta
///   A MonoDelta object representing the specified time interval.
/// @return A MonoTime object representing the resulting point in time.
MonoTime  operator-(const MonoTime& t, const MonoDelta& delta);

/// Compute the time interval between the specified points in time.
///
/// Semantically, this is equivalent to t0.GetDeltaSince(t1).
///
/// @param [in] t_end
///   The second point in time.  Semantically corresponds to the end
///   of the resulting time interval.
/// @param [in] t_beg
///   The first point in time.  Semantically corresponds to the beginning
///   of the resulting time interval.
/// @return A MonoDelta object representing the time interval between the
///   specified points in time.
MonoDelta  operator-(const MonoTime& t_end, const MonoTime& t_begin);
///@}

} // namespace mprmpr

#endif

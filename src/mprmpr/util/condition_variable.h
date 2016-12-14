#ifndef ANT_UTIL_CONDITION_VARIABLE_H_
#define ANT_UTIL_CONDITION_VARIABLE_H_
#include <pthread.h>

#include "mprmpr/util/monotime.h"
#include "mprmpr/util/mutex.h"

namespace mprmpr {

class ConditionVarImpl;
class TimeDelta;

class ConditionVariable {
 public:
  explicit ConditionVariable(Mutex* user_lock);

  ~ConditionVariable();

  void Wait() const;
  
  bool TimedWait(const MonoDelta& max_time) const;
  
  void Broadcast();
  void Signal();
  
 private:
 
  mutable pthread_cond_t condition_;
  pthread_mutex_t* user_mutex_;
  

  DISALLOW_COPY_AND_ASSIGN(ConditionVariable);
};

} // namespace mprmpr
#endif // ANT_UTIL_CONDITION_VARIABLE_H_

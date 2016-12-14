#include "mprmpr/util/mutex.h"

#include <glog/logging.h>

#include "mprmpr/base/walltime.h"

namespace mprmpr {

Mutex::Mutex()
{
  pthread_mutex_init(&native_handle_, NULL);
}

Mutex::~Mutex() {
  int rv = pthread_mutex_destroy(&native_handle_);
  DCHECK_EQ(0, rv) << ". " << strerror(rv);
}

bool Mutex::TryAcquire() {
  int rv = pthread_mutex_trylock(&native_handle_);
  return rv == 0;
}

void Mutex::Acquire() {
  if (PREDICT_TRUE(TryAcquire())) {
    return;
  }

  MicrosecondsInt64 start_time = GetMonoTimeMicros();
  int rv = pthread_mutex_lock(&native_handle_);
  DCHECK_EQ(0, rv) << ". " << strerror(rv)
  ; // NOLINT(whitespace/semicolon)
  MicrosecondsInt64 end_time = GetMonoTimeMicros();

  int64_t wait_time = end_time - start_time;
  if (wait_time > 0) {
  ;
  }

}

void Mutex::Release() {
  int rv = pthread_mutex_unlock(&native_handle_);
  DCHECK_EQ(0, rv) << ". " << strerror(rv);
}

} // namespace mprmpr

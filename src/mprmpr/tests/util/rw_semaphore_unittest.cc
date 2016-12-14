#include <gtest/gtest.h>
#include <mutex>
#include <thread>
#include <vector>

#include "mprmpr/util/locks.h"
#include "mprmpr/util/monotime.h"
#include "mprmpr/util/rw_semaphore.h"

using std::thread;
using std::vector;

namespace mprmpr {
struct SharedState {
  SharedState() : done(false), int_var(0) {}

  bool done;
  int64_t int_var;
  rw_semaphore sem;
};

// Thread which increases the value in the shared state under the write lock.
void Writer(SharedState* state) {
  int i = 0;
  while (true) {
    std::lock_guard<rw_semaphore> l(state->sem);
    state->int_var += (i++);
    if (state->done) {
      break;
    }
  }
}

// Thread which verifies that the value in the shared state only increases.
void Reader(SharedState* state) {
  int prev_val = 0;
  while (true) {
    shared_lock<rw_semaphore> l(state->sem);
    // The int var should only be seen to increase.
    CHECK_GE(state->int_var, prev_val);
    prev_val = state->int_var;
    if (state->done) {
      break;
    }
  }
}

// Test which verifies basic functionality of the semaphore.
// When run under TSAN this also verifies the barriers.
TEST(RWSemaphoreTest, TestBasicOperation) {
  SharedState s;
  vector<thread*> threads;
  // Start 5 readers and writers.
  for (int i = 0; i < 5; i++) {
    threads.push_back(new thread(Reader, &s));
    threads.push_back(new thread(Writer, &s));
  }

  // Let them contend for a short amount of time.
  SleepFor(MonoDelta::FromMilliseconds(50));

  // Signal them to stop.
  {
    std::lock_guard<rw_semaphore> l(s.sem);
    s.done = true;
  }

  for (thread* t : threads) {
    t->join();
    delete t;
  }
}

} // namespace mprmpr

#ifndef ANT_UTIL_FAILURE_DETECTOR_H_
#define ANT_UTIL_FAILURE_DETECTOR_H_
#include <string>
#include <unordered_map>

#include "mprmpr/base/callback.h"
#include "mprmpr/base/gscoped_ptr.h"
#include "mprmpr/base/macros.h"
#include "mprmpr/base/ref_counted.h"

#include "mprmpr/util/countdown_latch.h"
#include "mprmpr/util/monotime.h"
#include "mprmpr/util/locks.h"
#include "mprmpr/util/random.h"
#include "mprmpr/util/status_callback.h"

namespace mprmpr {

class MonoDelta;
class MonoTime;
class Status;
class Thread;

class FailureDetector : public base::RefCountedThreadSafe<FailureDetector> {
 public:
  enum NodeStatus {
    DEAD,
    ALIVE,
  };
  typedef std::unordered_map<std::string, NodeStatus> StatusMap;
  typedef base::Callback<void(const std::string& name,
                              const Status& status)> FailureDetectedCallback;

  virtual ~FailureDetector() {}

  virtual Status Track(const std::string& name,
                       const MonoTime& now,
                       const FailureDetectedCallback& callback) = 0;
  virtual Status UnTrack(const std::string& name) = 0;
  virtual bool IsTracking(const std::string& name) = 0;
  virtual Status MessageFrom(const std::string& name, const MonoTime& now) = 0;
  virtual void CheckForFailures(const MonoTime& now) = 0;

};

class TimedFailureDetector : public FailureDetector {
 public:
  struct Node {
    std::string permanent_name;
    MonoTime last_heard_of;
    FailureDetectedCallback callback;
    NodeStatus status;
  };

  explicit TimedFailureDetector(MonoDelta failure_period);
  virtual ~TimedFailureDetector();

  virtual Status Track(const std::string& name,
                       const MonoTime& now,
                       const FailureDetectedCallback& callback) override;
  virtual Status UnTrack(const std::string& name) override;
  virtual bool IsTracking(const std::string& name) override;
  virtual Status MessageFrom(const std::string& name, const MonoTime& now) override;
  virtual void CheckForFailures(const MonoTime& now) override;

 private:
  typedef std::unordered_map<std::string, Node*> NodeMap;
  FailureDetector::NodeStatus GetNodeStatusUnlocked(const std::string& name,
                                                    const MonoTime& now);
  const MonoDelta failure_period_;
  mutable simple_spinlock lock_;
  NodeMap nodes_;

  DISALLOW_COPY_AND_ASSIGN(TimedFailureDetector);
};

class RandomizedFailureMonitor {
 public:
  static const int64_t kMinWakeUpTimeMillis;

  RandomizedFailureMonitor(uint32_t random_seed,
                           int64_t period_mean_millis,
                           int64_t period_stddev_millis);
  virtual ~RandomizedFailureMonitor();

  Status Start();
  void Shutdown();
  Status MonitorFailureDetector(const std::string& name,
                                const scoped_refptr<FailureDetector>& fd);
  Status UnMonitorFailureDetector(const std::string& name);

 private:
  typedef std::unordered_map<std::string, scoped_refptr<FailureDetector>> FDMap;
  void RunThread();
  
  const int64_t period_mean_millis_;
  const int64_t period_stddev_millis_;
  ThreadSafeRandom random_;

  scoped_refptr<Thread> thread_;
  CountDownLatch run_latch_;

  mutable simple_spinlock lock_;
  FDMap fds_;
  bool shutdown_;

  DISALLOW_COPY_AND_ASSIGN(RandomizedFailureMonitor);
};

} // namespace mprmpr
#endif // ANT_UTIL_FAILURE_DETECTOR_H_

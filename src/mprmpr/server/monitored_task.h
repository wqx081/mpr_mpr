#ifndef KUDU_MONITORED_TASK_H
#define KUDU_MONITORED_TASK_H

#include <string>

#include "mprmpr/base/ref_counted.h"
#include "mprmpr/util/monotime.h"

namespace mprmpr {

class MonitoredTask : public base::RefCountedThreadSafe<MonitoredTask> {
 public:
  virtual ~MonitoredTask() {}

    enum State {
      kStatePreparing,
      kStateRunning,
      kStateComplete,
      kStateFailed,
      kStateAborted,
    };

    // Abort the ongoing task.
    virtual void Abort() = 0;

    // Task State
    virtual State state() const = 0;

    // Task Type Identifier
    virtual std::string type_name() const = 0;

    // Task description
    virtual std::string description() const = 0;

    // Task start time, may be !Initialized()
    virtual MonoTime start_timestamp() const = 0;

    // Task completion time, may be !Initialized()
    virtual MonoTime completion_timestamp() const = 0;
};

} // namespace mprmpr

#endif

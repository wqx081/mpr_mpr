#ifndef ANT_SERVER_WORKER_NODE_HEARTBEATER_H_
#define ANT_SERVER_WORKER_NODE_HEARTBEATER_H_

#include <memory>
#include <string>
#include <vector>

#include "mprmpr/base/macros.h"
#include "mprmpr/util/status.h"

namespace mprmpr {

namespace master {
class WorkerStatusPB;
} // namespace master

namespace worker_server {

class WorkerServer;
struct WorkerServerOptions;

// 心跳器
// 定时的向Master 发起心跳包，以及报告当前 Worker Server 的负载情况.
//
class Heartbeater {
 public:
  Heartbeater(const WorkerServerOptions& options, WorkerServer* server);
  Status Start();
  Status Stop();
  ~Heartbeater();

 private:
  class Thread;
  std::vector<std::unique_ptr<Thread>> threads_;
  DISALLOW_COPY_AND_ASSIGN(Heartbeater);
};

} // namespace worker_server
} // namespace mprmpr
#endif // ANT_SERVER_WORKER_NODE_HEARTBEATER_H_

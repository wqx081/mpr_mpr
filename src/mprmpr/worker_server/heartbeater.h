#ifndef ANT_SERVER_WORKER_NODE_HEARTBEATER_H_
#define ANT_SERVER_WORKER_NODE_HEARTBEATER_H_

#include <memory>
#include <string>
#include <vector>

#include "mprmpr/base/macros.h"
#include "mprmpr/util/status.h"

namespace mprmpr {

namespace master {
class WorkerReportPB;
} // namespace master

namespace worker_server {

class WorkerServer;
struct WorkerServerOptions;

class Heartbeater {
 public:
  Heartbeater(const WorkerServerOptions& options, WorkerServer* server);
  Status Start();
  Status Stop();
  void TriggerASAP();
  //TODO(wqx):
  //
  ~Heartbeater();

  std::vector<master::WorkerReportPB> GenerateIncrementalWorkerReportsForTests();
  std::vector<master::WorkerReportPB> GenerateFullWorkerReportsForTests();
  void MarkWorkerReportsAcknowledgedForTests(
      const std::vector<master::WorkerReportPB>& reports);

 private:
  class Thread;
  std::vector<std::unique_ptr<Thread>> threads_;
  DISALLOW_COPY_AND_ASSIGN(Heartbeater);
};

} // namespace worker_server
} // namespace mprmpr
#endif // ANT_SERVER_WORKER_NODE_HEARTBEATER_H_

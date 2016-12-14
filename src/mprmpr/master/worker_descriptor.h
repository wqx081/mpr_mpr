#ifndef MPRMPR_MASTER_WORKER_DESCRIPTOR_H_
#define MPRMPR_MASTER_WORKER_DESCRIPTOR_H_

#include <memory>
#include <mutex>
#include <string>

#include "mprmpr/base/gscoped_ptr.h"
#include "mprmpr/util/locks.h"
#include "mprmpr/util/make_shared.h"
#include "mprmpr/util/monotime.h"
#include "mprmpr/util/status.h"

namespace mprmpr {

class NodeInstancePB;
class WorkerStatusPB;
class ServerRegistrationPB;


namespace rpc {
class Messenger;
} // namespace rpc

namespace master {

// Master 端表示单个 worker server.
//
// 跟踪最后的心跳，状态，instance 标识符， 等等
// 该类为线程安全
class WorkerDescriptor {
 public:
  static Status RegisterNew(const NodeInstancePB& instance,
                            const ServerRegistrationPB& registration,
                            const WorkerStatusPB& worker_status,
                            std::shared_ptr<WorkerDescriptor>* desc);
  virtual ~WorkerDescriptor();
  void UpdateHearbeatTime();
  void UpdateWorkerStatus(const WorkerStatusPB& worker_status);
  
  MonoDelta TimeSinceHeartbeat() const;
  bool PresumedDead() const;

  Status Register(const NodeInstancePB& instance,
                  const ServerRegistrationPB& registration,
                  const WorkerStatusPB& worker_status);

  const std::string& permanent_uuid() const;
  int64_t latest_seqno() const;

  void GetRegistration(ServerRegistrationPB* reg) const;
  void GetNodeInstancePB(NodeInstancePB* instance_pb) const;
  void GetWorkerStatusPB(WorkerStatusPB* worker_status_pb) const;

  std::string ToString() const;

 private:
  explicit WorkerDescriptor(std::string perm_id);

  mutable simple_spinlock lock_;

  const std::string permanent_uuid_;
  int64_t latest_seqno_;

  MonoTime last_heartbeat_;
  gscoped_ptr<ServerRegistrationPB> registration_;
  gscoped_ptr<WorkerStatusPB> worker_status_;

  ALLOW_MAKE_SHARED(WorkerDescriptor);
  DISALLOW_COPY_AND_ASSIGN(WorkerDescriptor);
};

} // namespace master
} // namespace mprmpr
#endif // MPRMPR_MASTER_WORKER_DESCRIPTOR_H_

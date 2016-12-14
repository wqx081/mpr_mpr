#ifndef MPRMPR_MASTER_WORKER_MANAGER_H_
#define MPRMPR_MASTER_WORKER_MANAGER_H_

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "mprmpr/base/macros.h"
#include "mprmpr/util/locks.h"
#include "mprmpr/util/monotime.h"
#include "mprmpr/util/status.h"

namespace mprmpr {

class NodeInstancePB;
class ServerRegistrationPB;
class WorkerStatusPB;

namespace master {

class WorkerDescriptor;
using WorkerDescriptorVector = std::vector<std::shared_ptr<WorkerDescriptor>>;


class WorkerManager {
 public:
  WorkerManager();
  virtual ~WorkerManager();

  Status LookupWorker(const NodeInstancePB& instance,
                      std::shared_ptr<WorkerDescriptor>* desc) const;
  bool LookupWorkerByUUID(const std::string& uuid,
                          std::shared_ptr<WorkerDescriptor>* desc) const;

  Status RegisterWorker(const NodeInstancePB& instance,
                        const ServerRegistrationPB& registration,
                        const WorkerStatusPB& worker_status,
                        std::shared_ptr<WorkerDescriptor>* desc);
  void GetAllDescriptors(std::vector<std::shared_ptr<WorkerDescriptor>>* descs) const;
  void GetAllLiveDescriptors(std::vector<std::shared_ptr<WorkerDescriptor>>* descs) const;

  int GetCount() const;

 private:
  mutable rw_spinlock lock_;

  using WorkerDescriptorMap = std::unordered_map<std::string,
                                                 std::shared_ptr<WorkerDescriptor>>;
  WorkerDescriptorMap servers_by_id_;

  DISALLOW_COPY_AND_ASSIGN(WorkerManager);
};

} // namespace master
} // namespace mprmpr
#endif // MPRMPR_MASTER_WORKER_MANAGER_H_

#include "mprmpr/master/worker_manager.h"

#include <mutex>

#include "mprmpr/base/map-util.h"
#include "mprmpr/base/strings/substitute.h"
#include "mprmpr/master/master.pb.h"
#include "mprmpr/master/worker_descriptor.h"

namespace mprmpr {
namespace master {

WorkerManager::WorkerManager() {
}

WorkerManager::~WorkerManager() {
}

Status WorkerManager::LookupWorker(const NodeInstancePB& instance,
                                   std::shared_ptr<WorkerDescriptor>* desc) const {
  shared_lock<rw_spinlock> l(lock_);
  const std::shared_ptr<WorkerDescriptor>* found_ptr = 
    FindOrNull(servers_by_id_, instance.permanent_uuid());
  if (!found_ptr) {
    return Status::NotFound("Unknown worker server ID", instance.ShortDebugString());
  }
  const std::shared_ptr<WorkerDescriptor> found = *found_ptr;

  if (instance.instance_seqno() != found->latest_seqno()) {
    return Status::NotFound("Mismatched instance sequence number", instance.ShortDebugString());
  }

  *desc = found;
  return Status::OK();
}

bool WorkerManager::LookupWorkerByUUID(const std::string& uuid,
                                       std::shared_ptr<WorkerDescriptor>* desc) const {
  shared_lock<rw_spinlock> l(lock_);
  return FindCopy(servers_by_id_, uuid, desc);
}

Status WorkerManager::RegisterWorker(const NodeInstancePB& instance,
                                     const ServerRegistrationPB& registration,
                                     const WorkerStatusPB& worker_status,
                                     std::shared_ptr<WorkerDescriptor>* desc) {
  std::lock_guard<rw_spinlock> l(lock_);
  const std::string& uuid = instance.permanent_uuid();

  if (!ContainsKey(servers_by_id_, uuid)) {
    std::shared_ptr<WorkerDescriptor> new_desc;
    RETURN_NOT_OK(WorkerDescriptor::RegisterNew(instance, registration, worker_status, &new_desc));
    InsertOrDie(&servers_by_id_, uuid, new_desc);
    LOG(INFO) << strings::Substitute("Registered new tserver with Master: $0", new_desc->ToString());
    desc->swap(new_desc);
  } else {
    std::shared_ptr<WorkerDescriptor> found(FindOrDie(servers_by_id_, uuid));
    RETURN_NOT_OK(found->Register(instance, registration, worker_status));
    LOG(INFO) << strings::Substitute("Re-registered known tserver with Master: $0", found->ToString());
    desc->swap(found);
  }

  return Status::OK();
}

void WorkerManager::GetAllDescriptors(std::vector<std::shared_ptr<WorkerDescriptor> > *descs) const {
  descs->clear();
  shared_lock<rw_spinlock> l(lock_);
  AppendValuesFromMap(servers_by_id_, descs);
}

void WorkerManager::GetAllLiveDescriptors(std::vector<std::shared_ptr<WorkerDescriptor> > *descs) const {
  descs->clear();

  shared_lock<rw_spinlock> l(lock_);
  descs->reserve(servers_by_id_.size());
  for (const WorkerDescriptorMap::value_type& entry : servers_by_id_) {
    const std::shared_ptr<WorkerDescriptor>& worker = entry.second;
    if (!worker->PresumedDead()) {
      descs->push_back(worker);
    }
  }
}

int WorkerManager::GetCount() const {
  shared_lock<rw_spinlock> l(lock_);
  return servers_by_id_.size();
}

} // namespace master
} // namespace mprmpr

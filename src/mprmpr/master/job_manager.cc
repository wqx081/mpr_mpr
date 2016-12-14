#include "mprmpr/master/job_manager.h"
#include "mprmpr/util/locks.h"

namespace mprmpr {

JobManager::JobManager() {}

JobManager* JobManager::get() {
  return Singleton<JobManager>::get();
}

void JobManager::AddJobDescriptor(const JobDescriptorPB* job_desc) {
  std::lock_guard<simple_spinlock> l(lock_);
  jobs_.push_back(job_desc);
}

} // namespace mprmpr

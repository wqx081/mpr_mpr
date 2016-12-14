#ifndef MPRMPR_MASTER_JOB_MANAGER_H_
#define MPRMPR_MASTER_JOB_MANAGER_H_

#include "mprmpr/base/macros.h"
#include "mprmpr/base/singleton.h"
#include "mprmpr/common/common.pb.h"
#include "mprmpr/util/locks.h"

#include <mutex>

namespace mprmpr {


class JobManager {
 public:
  static JobManager* get();
  void AddJobDescriptor(const JobDescriptorPB* job_descriptor);

 private:
  friend class Singleton<JobManager>;
  JobManager();

  mutable simple_spinlock lock_;
  std::vector<const JobDescriptorPB*> jobs_;

  DISALLOW_COPY_AND_ASSIGN(JobManager);
};

} // namespace mprmpr
#endif // MPRMPR_MASTER_JOB_MANAGER_H_

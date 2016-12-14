#include "mprmpr/master/worker_descriptor.h"

#include <math.h>
#include <mutex>
#include <unordered_set>
#include <vector>

#include "mprmpr/common/common.h"
#include "mprmpr/master/master.pb.h"
#include "mprmpr/base/strings/substitute.h"
#include "mprmpr/util/net/net_util.h"

#include <gflags/gflags.h>

DEFINE_int32(worker_unresponsive_timeout_ms, 60 * 1000, "");

namespace mprmpr {
namespace master {

Status WorkerDescriptor::RegisterNew(const NodeInstancePB& instance,
                                     const ServerRegistrationPB& registration,
                                     const WorkerStatusPB& worker_status,
                                     std::shared_ptr<WorkerDescriptor>* desc) {
  std::shared_ptr<WorkerDescriptor> ret(std::make_shared<WorkerDescriptor>(instance.permanent_uuid()));
  RETURN_NOT_OK(ret->Register(instance, registration, worker_status));
  desc->swap(ret);
  return Status::OK();
}

WorkerDescriptor::WorkerDescriptor(std::string perm_id)
    : permanent_uuid_(std::move(perm_id)),
      latest_seqno_(-1),
      last_heartbeat_(MonoTime::Now()) {
}

WorkerDescriptor::~WorkerDescriptor() {
}

namespace {

bool HostPortPBsEqual(const google::protobuf::RepeatedPtrField<HostPortPB>& pb1,
                      const google::protobuf::RepeatedPtrField<HostPortPB>& pb2) {
  if (pb1.size() != pb2.size()) {
    return false;
  }

  std::unordered_set<HostPort, HostPortHasher, HostPortEqualityPredicate> hostports1;
  std::unordered_set<HostPort, HostPortHasher, HostPortEqualityPredicate> hostports2;
  for (int i = 0; i < pb1.size(); ++i) {
    HostPort hp1;
    HostPort hp2;

    if (!HostPortFromPB(pb1.Get(i), &hp1).ok()) {
      return false;
    }
    if (!HostPortFromPB(pb2.Get(i), &hp2).ok()) {
      return false;
    }
    hostports1.insert(hp1);
    hostports2.insert(hp2);
  }
  return hostports1 == hostports2;
}

} // namespace

// 注册，初始化WorkerDescriptor
Status WorkerDescriptor::Register(const NodeInstancePB& instance,
                                  const ServerRegistrationPB& registration,
                                  const WorkerStatusPB& worker_status) {
  std::lock_guard<simple_spinlock> l(lock_);
  CHECK_EQ(instance.permanent_uuid(), permanent_uuid_);

  // 不允许 Worker Node 改变 IP地址
  if (registration_ && (!HostPortPBsEqual(registration_->rpc_addresses(), registration.rpc_addresses()) ||
                        !HostPortPBsEqual(registration_->http_addresses(), registration.http_addresses()))) {
    std::string msg = strings::Substitute("Worker server $0 is attempting to re-register with a different host/host."
                                          "This is not currently supported. Old: {$1} New: {$2}",
                                          instance.permanent_uuid(),
                                          registration_->ShortDebugString(),
                                          registration_->ShortDebugString());
    LOG(ERROR) << msg;
    return Status::InvalidArgument(msg);
  }

  if (registration.rpc_addresses().empty() || registration.http_addresses().empty()) {
    return Status::InvalidArgument("Invalid registration: must have at least one RPC and one HTTP address",
                                   registration.ShortDebugString());
  }

  if (instance.instance_seqno() < latest_seqno_) {
    return Status::AlreadyPresent(
            strings::Substitute("Cannot register with sequence number $0: Already have a registration from "
                                  "sequence number: $1", instance.instance_seqno(), latest_seqno_));
  } else if (instance.instance_seqno() == latest_seqno_) {
    LOG(INFO) << "Processing retry of Worker registration from " << instance.ShortDebugString();
  }

  latest_seqno_ = instance.instance_seqno();
  registration_.reset(new ServerRegistrationPB(registration));
  worker_status_.reset(new WorkerStatusPB(worker_status));

  return Status::OK();
}

void WorkerDescriptor::UpdateHearbeatTime() {
  std::lock_guard<simple_spinlock> l(lock_);
  last_heartbeat_ = MonoTime::Now();
}

void WorkerDescriptor::UpdateWorkerStatus(const WorkerStatusPB& worker_status) {
  std::lock_guard<simple_spinlock> l(lock_);
  worker_status_->CopyFrom(worker_status);
}

MonoDelta WorkerDescriptor::TimeSinceHeartbeat() const {
  MonoTime now(MonoTime::Now());
  std::lock_guard<simple_spinlock> l(lock_);
  return now - last_heartbeat_;
}

// 心跳停止超过一分钟，我们认为worker server 已死
bool WorkerDescriptor::PresumedDead() const {
  return TimeSinceHeartbeat().ToMilliseconds() >= FLAGS_worker_unresponsive_timeout_ms;
}

int64_t WorkerDescriptor::latest_seqno() const {
  std::lock_guard<simple_spinlock> l(lock_);
  return latest_seqno_;
}

void WorkerDescriptor::GetRegistration(ServerRegistrationPB* reg) const {
  std::lock_guard<simple_spinlock> l(lock_);
  CHECK(registration_) << "No registration";
  CHECK_NOTNULL(reg)->CopyFrom(*registration_);
}

void WorkerDescriptor::GetNodeInstancePB(NodeInstancePB* instance) const {
  std::lock_guard<simple_spinlock> l(lock_);
  instance->set_permanent_uuid(permanent_uuid_);
  instance->set_instance_seqno(latest_seqno_);
}

void WorkerDescriptor::GetWorkerStatusPB(WorkerStatusPB* worker_status) const {
  std::lock_guard<simple_spinlock> l(lock_);
  CHECK(worker_status_) << "No worker status";
  CHECK_NOTNULL(worker_status)->CopyFrom(*worker_status_);
}

std::string WorkerDescriptor::ToString() const {
  std::lock_guard<simple_spinlock> l(lock_);
  const auto& addr = registration_->rpc_addresses(0);
  return strings::Substitute("$0 ($1:$2): $3",
                             permanent_uuid_,
                             addr.host(),
                             addr.port(),
                             worker_status_->ShortDebugString());
}

} // namespace master
} // namespace mprmpr

#ifndef MPRMPR_MASTER_MASTER_SERVICE_IMPL_H_
#define MPRMPR_MASTER_MASTER_SERVICE_IMPL_H_

#include "mprmpr/base/macros.h"
#include "mprmpr/master/master.service.pb.h"
#include "mprmpr/util/metrics.h"

namespace mprmpr {

class NodeInstancePB;

namespace master {

class Master;
class WorkerDescriptor;

class MasterServiceImpl : public MasterServiceIf {
 public:
  explicit MasterServiceImpl(Master* server);
  virtual void Ping(const PingRequestPB* req,
                    PingResponsePB* resp,
                    rpc::RpcContext* rpc) override;
  virtual void WorkerHeartbeat(const WorkerHeartbeatRequestPB* req,
                               WorkerHeartbeatResponsePB* resp,
                               rpc::RpcContext* rpc) override;
 private:
  Master* server_;
  DISALLOW_COPY_AND_ASSIGN(MasterServiceImpl);
};

} // namespace master
} // namespace mprmpr
#endif // MPRMPR_MASTER_MASTER_SERVICE_IMPL_H_

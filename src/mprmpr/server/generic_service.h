#ifndef KUDU_SERVER_GENERIC_SERVICE_H
#define KUDU_SERVER_GENERIC_SERVICE_H

#include "mprmpr/base/macros.h"
#include "mprmpr/server/server_base.service.pb.h"

namespace mprmpr {
namespace server {

class ServerBase;

class GenericServiceImpl : public GenericServiceIf {
 public:
  explicit GenericServiceImpl(ServerBase* server);
  virtual ~GenericServiceImpl();

  virtual void SetFlag(const SetFlagRequestPB* req,
                       SetFlagResponsePB* resp,
                       rpc::RpcContext* rpc) OVERRIDE;

  virtual void FlushCoverage(const FlushCoverageRequestPB* req,
                             FlushCoverageResponsePB* resp,
                             rpc::RpcContext* rpc) OVERRIDE;

  virtual void ServerClock(const ServerClockRequestPB* req,
                           ServerClockResponsePB* resp,
                           rpc::RpcContext* rpc) OVERRIDE;

  virtual void SetServerWallClockForTests(const SetServerWallClockForTestsRequestPB *req,
                                          SetServerWallClockForTestsResponsePB *resp,
                                          rpc::RpcContext *context) OVERRIDE;

  virtual void GetStatus(const GetStatusRequestPB* req,
                         GetStatusResponsePB* resp,
                         rpc::RpcContext* rpc) OVERRIDE;
 private:
  ServerBase* server_;

  DISALLOW_COPY_AND_ASSIGN(GenericServiceImpl);
};

} // namespace server
} // namespace mprmpr

#endif /* KUDU_SERVER_GENERIC_SERVICE_H */

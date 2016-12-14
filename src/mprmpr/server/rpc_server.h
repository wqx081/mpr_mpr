#ifndef ANT_SERVER_RPC_SERVER_H_
#define ANT_SERVER_RPC_SERVER_H_

#include <memory>
#include <vector>

#include "mprmpr/base/gscoped_ptr.h"
#include "mprmpr/util/net/sockaddr.h"
#include "mprmpr/util/status.h"

namespace mprmpr {

namespace rpc {

class AcceptorPool;
class Messenger;
class ServiceIf;
class ServicePool;

} // namespace rpc

struct RpcServerOptions {
  RpcServerOptions();

  std::string rpc_bind_addresses;
  uint32_t num_acceptors_per_address;
  uint32_t num_service_threads;
  uint16_t default_port;
  size_t service_queue_length;
};

class RpcServer {
 public:
  explicit RpcServer(const RpcServerOptions& opts);
  ~RpcServer();

  Status Init(const std::shared_ptr<rpc::Messenger>& messenger);  
  Status RegisterService(gscoped_ptr<rpc::ServiceIf> service);
  Status Bind();
  Status Start();
  void Shutdown();

  std::string ToString() const;

  Status GetBoundAddresses(std::vector<Sockaddr>* addresses) const;
  const rpc::ServicePool* service_pool(const std::string& service_name) const;

 private:
  enum ServerState {
    UNINITIALIZED,
    INITIALIZED,
    BOUND,
    STARTED
  };
  ServerState server_state_;

  const RpcServerOptions options_;
  std::shared_ptr<rpc::Messenger> messenger_;
  std::vector<Sockaddr> rpc_bind_addresses_;
  std::vector<std::shared_ptr<rpc::AcceptorPool>> acceptor_pools_;

  DISALLOW_COPY_AND_ASSIGN(RpcServer);
};

} // namespace mprmpr
#endif // ANT_SERVER_RPC_SERVER_H_

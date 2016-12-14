#include "mprmpr/server/rpc_server.h"

#include <list>
#include <string>

#include <gflags/gflags.h>

#include "mprmpr/base/casts.h"
#include "mprmpr/base/gscoped_ptr.h"
#include "mprmpr/base/strings/substitute.h"
#include "mprmpr/rpc/acceptor_pool.h"
#include "mprmpr/rpc/messenger.h"
#include "mprmpr/rpc/service_if.h"
#include "mprmpr/rpc/service_pool.h"
#include "mprmpr/util/net/net_util.h"

DEFINE_string(rpc_bind_addresses, "0.0.0.0",
              "Comma-separated list of addresses to bind to for RPC connections. "
              "Currently, ephemeral ports (i.e. port 0) are not allowed.");

DEFINE_int32(rpc_num_acceptors_per_address, 1,
             "Number of RPC acceptor threads for each bound address");

DEFINE_int32(rpc_num_service_threads, 10,
             "Number of RPC worker threads to run");

DEFINE_int32(rpc_service_queue_length, 50,
             "Default length of queue for incoming RPC requests");

DEFINE_bool(rpc_server_allow_ephemeral_ports, false,
            "Allow binding to ephemeral ports. This can cause problems, so currently "
            "only allowed in tests.");

namespace mprmpr {

RpcServerOptions::RpcServerOptions()
    : rpc_bind_addresses(FLAGS_rpc_bind_addresses),
      num_acceptors_per_address(FLAGS_rpc_num_acceptors_per_address),
      num_service_threads(FLAGS_rpc_num_service_threads),
      default_port(0),
      service_queue_length(FLAGS_rpc_service_queue_length) {
}

///////////////////////////////////


RpcServer::RpcServer(const RpcServerOptions& opts)
    : server_state_(UNINITIALIZED),
      options_(std::move(opts)) {}

RpcServer::~RpcServer() {
  Shutdown();
}

std::string RpcServer::ToString() const {
  //TODO(wqx):
  return "RpcServer";
}

Status RpcServer::Init(const std::shared_ptr<rpc::Messenger>& messenger) {
  CHECK_EQ(server_state_, UNINITIALIZED);
  messenger_ = messenger;

  RETURN_NOT_OK(ParseAddressList(options_.rpc_bind_addresses,
                                 options_.default_port,
                                 &rpc_bind_addresses_));
  for (const Sockaddr& addr : rpc_bind_addresses_) {
    if (IsPrivilegedPort(addr.port())) {
      LOG(WARNING) << "May be unable to bind to privileged port for address "
                   << addr.ToString();
    }

    if (addr.port() == 0 && !FLAGS_rpc_server_allow_ephemeral_ports) {
      LOG(FATAL) << "Binding to ephemeral ports not supported (RPC address "
                 << "configured to " << addr.ToString() << ")";
    }
  }
  
  server_state_ = INITIALIZED;
  return Status::OK();
}

Status RpcServer::RegisterService(gscoped_ptr<rpc::ServiceIf> service) {
  CHECK(server_state_ == INITIALIZED ||
        server_state_ == BOUND) << "bad state: " << server_state_;
  const scoped_refptr<MetricEntity>& metric_entity = messenger_->metric_entity();
  string service_name = service->service_name();
  scoped_refptr<rpc::ServicePool> service_pool =
    new rpc::ServicePool(std::move(service), metric_entity, options_.service_queue_length);
  RETURN_NOT_OK(service_pool->Init(options_.num_service_threads));
  RETURN_NOT_OK(messenger_->RegisterService(service_name, service_pool));
  return Status::OK();
}

Status RpcServer::Bind() {
  CHECK_EQ(server_state_, INITIALIZED);

  std::vector<std::shared_ptr<rpc::AcceptorPool>> new_acceptor_pools;
  for (const Sockaddr& bind_addr : rpc_bind_addresses_) {
    std::shared_ptr<rpc::AcceptorPool> pool;
    RETURN_NOT_OK(messenger_->AddAcceptorPool(bind_addr,
                                              &pool));
    new_acceptor_pools.push_back(pool);
  }
  acceptor_pools_.swap(new_acceptor_pools);

  server_state_ = BOUND;
  return Status::OK();
}

Status RpcServer::Start() {
  if (server_state_ == INITIALIZED) {
    RETURN_NOT_OK(Bind());
  } 
  CHECK_EQ(server_state_, BOUND);
  server_state_ = STARTED;

  for (const std::shared_ptr<rpc::AcceptorPool>& pool : acceptor_pools_) {
    RETURN_NOT_OK(pool->Start(options_.num_acceptors_per_address));
  }

  std::vector<Sockaddr> bound_addrs;
  RETURN_NOT_OK(GetBoundAddresses(&bound_addrs));
  std::string bound_addrs_str;
  for (const Sockaddr& bind_addr : bound_addrs) {
    if (!bound_addrs_str.empty()) bound_addrs_str += ", ";
    bound_addrs_str += bind_addr.ToString();
  }
  LOG(INFO) << "RPC server started. Bound to: " << bound_addrs_str;

  return Status::OK();
}

void RpcServer::Shutdown() {
  for (const std::shared_ptr<rpc::AcceptorPool>& pool : acceptor_pools_) {
    pool->Shutdown();
  }
  acceptor_pools_.clear();

  if (messenger_) {
    WARN_NOT_OK(messenger_->UnregisterAllServices(), "Unable to unregister our services");
  }
}

Status RpcServer::GetBoundAddresses(vector<Sockaddr>* addresses) const {
  CHECK(server_state_ == BOUND ||
                server_state_ == STARTED) << "bad state: " << server_state_;
  for (const std::shared_ptr<rpc::AcceptorPool>& pool : acceptor_pools_) {
    Sockaddr bound_addr;
    RETURN_NOT_OK_PREPEND(pool->GetBoundAddress(&bound_addr),
                          "Unable to get bound address from AcceptorPool");
    addresses->push_back(bound_addr);
  }
  return Status::OK();
}

const rpc::ServicePool* RpcServer::service_pool(const string& service_name) const {
  return down_cast<rpc::ServicePool*>(messenger_->rpc_service(service_name).get());
}

} // namespace mprmpr

#ifndef KUDU_RPC_MESSENGER_H
#define KUDU_RPC_MESSENGER_H

#include <memory>
#include <stdint.h>
#include <unordered_map>

#include <list>
#include <string>
#include <vector>

#include <gtest/gtest_prod.h>

#include "mprmpr/base/gscoped_ptr.h"
#include "mprmpr/base/ref_counted.h"
#include "mprmpr/rpc/response_callback.h"
#include "mprmpr/util/locks.h"
#include "mprmpr/util/metrics.h"
#include "mprmpr/util/monotime.h"
#include "mprmpr/util/net/sockaddr.h"
#include "mprmpr/util/status.h"

namespace mprmpr {

class Socket;
class SSLFactory;
class ThreadPool;

namespace rpc {

class AcceptorPool;
class DumpRunningRpcsRequestPB;
class DumpRunningRpcsResponsePB;
class InboundCall;
class Messenger;
class OutboundCall;
class Reactor;
class ReactorThread;
class RpcService;
class RpczStore;

struct AcceptorPoolInfo {
 public:
  explicit AcceptorPoolInfo(Sockaddr bind_address)
      : bind_address_(std::move(bind_address)) {}

  Sockaddr bind_address() const {
    return bind_address_;
  }

 private:
  Sockaddr bind_address_;
};

class MessengerBuilder {
 public:
  friend class Messenger;
  friend class ReactorThread;

  explicit MessengerBuilder(std::string name);
  MessengerBuilder &set_connection_keepalive_time(const MonoDelta &keepalive);
  MessengerBuilder &set_num_reactors(int num_reactors);
  MessengerBuilder &set_min_negotiation_threads(int min_negotiation_threads);
  MessengerBuilder &set_max_negotiation_threads(int max_negotiation_threads);
  MessengerBuilder &set_coarse_timer_granularity(const MonoDelta &granularity);
  MessengerBuilder &set_metric_entity(const scoped_refptr<MetricEntity>& metric_entity);

  Status Build(std::shared_ptr<Messenger> *msgr);

 private:
  const std::string name_;
  MonoDelta connection_keepalive_time_;
  int num_reactors_;
  int min_negotiation_threads_;
  int max_negotiation_threads_;
  MonoDelta coarse_timer_granularity_;
  scoped_refptr<MetricEntity> metric_entity_;
};

class Messenger {
 public:
  friend class MessengerBuilder;
  friend class Proxy;
  friend class Reactor;
  typedef std::vector<std::shared_ptr<AcceptorPool> > acceptor_vec_t;
  typedef std::unordered_map<std::string, scoped_refptr<RpcService> > RpcServicesMap;

  static const uint64_t UNKNOWN_CALL_ID = 0;

  ~Messenger();

  void Shutdown();
  Status AddAcceptorPool(const Sockaddr &accept_addr,
                         std::shared_ptr<AcceptorPool>* pool);
  Status RegisterService(const std::string& service_name,
                         const scoped_refptr<RpcService>& service);
  Status UnregisterService(const std::string& service_name);
  Status UnregisterAllServices();
  void QueueOutboundCall(const std::shared_ptr<OutboundCall> &call);
  void QueueInboundCall(gscoped_ptr<InboundCall> call);
  void RegisterInboundSocket(Socket *new_socket, const Sockaddr &remote);
  Status DumpRunningRpcs(const DumpRunningRpcsRequestPB& req,
                         DumpRunningRpcsResponsePB* resp);
  void ScheduleOnReactor(const std::function<void(const Status&)>& func,
                         MonoDelta when);

  SSLFactory* ssl_factory() const { return ssl_factory_.get(); }
  ThreadPool* negotiation_pool() const { return negotiation_pool_.get(); }
  RpczStore* rpcz_store() { return rpcz_store_.get(); }
  int num_reactors() const { return reactors_.size(); }

  std::string name() const {
    return name_;
  }

  bool ssl_enabled() const { return ssl_enabled_; }

  bool closing() const {
    shared_lock<rw_spinlock> l(lock_.get_lock());
    return closing_;
  }

  scoped_refptr<MetricEntity> metric_entity() const { return metric_entity_.get(); }

  const scoped_refptr<RpcService> rpc_service(const std::string& service_name) const;

 private:
  FRIEND_TEST(TestRpc, TestConnectionKeepalive);

  explicit Messenger(const MessengerBuilder &bld);

  Reactor* RemoteToReactor(const Sockaddr &remote);
  Status Init();
  void RunTimeoutThread();
  void UpdateCurTime();

  void AllExternalReferencesDropped();

  const std::string name_;

  mutable percpu_rwlock lock_;

  bool closing_;

  bool ssl_enabled_;

  acceptor_vec_t acceptor_pools_;

  RpcServicesMap rpc_services_;

  std::vector<Reactor*> reactors_;

  gscoped_ptr<ThreadPool> negotiation_pool_;

  gscoped_ptr<SSLFactory> ssl_factory_;

  std::unique_ptr<RpczStore> rpcz_store_;

  scoped_refptr<MetricEntity> metric_entity_;

  // The ownership of the Messenger object is somewhat subtle. The pointer graph
  // looks like this:
  //
  //    [User Code ]             |      [ Internal code ]
  //                             |
  //     shared_ptr[1]           |
  //         |                   |
  //         v
  //      Messenger    <------------ shared_ptr[2] --- Reactor
  //       ^    |       ----------- bare pointer --> Reactor
  //        \__/
  //     shared_ptr[2]
  //     (retain_self_)
  //
  // shared_ptr[1] instances use Messenger::AllExternalReferencesDropped()
  //   as a deleter.
  // shared_ptr[2] are "traditional" shared_ptrs which call 'delete' on the
  //   object.
  //
  // The teardown sequence is as follows:
  // Option 1): User calls "Shutdown()" explicitly:
  //  - Messenger::Shutdown tells Reactors to shut down
  //  - When each reactor thread finishes, it drops its shared_ptr[2]
  //  - the Messenger::retain_self instance remains, keeping the Messenger
  //    alive.
  //  - The user eventually drops its shared_ptr[1], which calls
  //    Messenger::AllExternalReferencesDropped. This drops retain_self_
  //    and results in object destruction.
  // Option 2): User drops all of its shared_ptr[1] references
  //  - Though the Reactors still reference the Messenger, AllExternalReferencesDropped
  //    will get called, which triggers Messenger::Shutdown.
  //  - AllExternalReferencesDropped drops retain_self_, so the only remaining
  //    references are from Reactor threads. But the reactor threads are shutting down.
  //  - When the last Reactor thread dies, there will be no more shared_ptr[1] references
  //    and the Messenger will be destroyed.
  //
  // The main goal of all of this confusion is that the reactor threads need to be able
  // to shut down asynchronously, and we need to keep the Messenger alive until they
  // do so. So, handing out a normal shared_ptr to users would force the Messenger
  // destructor to Join() the reactor threads, which causes a problem if the user
  // tries to destruct the Messenger from within a Reactor thread itself.
  std::shared_ptr<Messenger> retain_self_;

  DISALLOW_COPY_AND_ASSIGN(Messenger);
};

} // namespace rpc
} // namespace mprmpr

#endif

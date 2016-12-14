#ifndef KUDU_RPC_CONNECTION_H
#define KUDU_RPC_CONNECTION_H

#include <boost/intrusive/list.hpp>
#include <ev++.h>
#include <memory>
#include <stdint.h>
#include <unordered_map>

#include <limits>
#include <string>
#include <vector>

#include "mprmpr/base/gscoped_ptr.h"
#include "mprmpr/base/ref_counted.h"
#include "mprmpr/rpc/outbound_call.h"
#include "mprmpr/rpc/sasl_client.h"
#include "mprmpr/rpc/sasl_server.h"
#include "mprmpr/rpc/inbound_call.h"
#include "mprmpr/rpc/transfer.h"
#include "mprmpr/util/monotime.h"
#include "mprmpr/util/net/sockaddr.h"
#include "mprmpr/util/net/socket.h"
#include "mprmpr/util/object_pool.h"
#include "mprmpr/util/status.h"

namespace mprmpr {
namespace rpc {

class DumpRunningRpcsRequestPB;
class RpcConnectionPB;
class ReactorThread;
class RpczStore;

class Connection : public base::RefCountedThreadSafe<Connection> {
 public:
  enum Direction {
    CLIENT,
    SERVER
  };

  Connection(ReactorThread *reactor_thread, Sockaddr remote, Socket* socket,
             Direction direction);
  Status SetNonBlocking(bool enabled);

  void EpollRegister(ev::loop_ref& loop);

  ~Connection();

  MonoTime last_activity_time() const {
    return last_activity_time_;
  }

  bool Idle() const;
  void Shutdown(const Status &status);
  void QueueOutboundCall(const std::shared_ptr<OutboundCall> &call);
  void QueueResponseForCall(gscoped_ptr<InboundCall> call);
  const Sockaddr &remote() const { return remote_; }
  void set_user_credentials(const UserCredentials &user_credentials);
  UserCredentials* mutable_user_credentials() { return &user_credentials_; }
  const UserCredentials &user_credentials() const { return user_credentials_; }
  RpczStore* rpcz_store();
  void ReadHandler(ev::io &watcher, int revents);
  void WriteHandler(ev::io &watcher, int revents);
  std::string ToString() const;

  Direction direction() const { return direction_; }

  Socket* socket() { return socket_.get(); }

  SaslClient &sasl_client() { return sasl_client_; }
  SaslServer &sasl_server() { return sasl_server_; }

  Status InitSSLIfNecessary();

  Status InitSaslClient();

  Status InitSaslServer();

  void CompleteNegotiation(const Status &negotiation_status);

  void MarkNegotiationComplete();

  Status DumpPB(const DumpRunningRpcsRequestPB& req,
                RpcConnectionPB* resp);

  ReactorThread *reactor_thread() const { return reactor_thread_; }

 private:
  friend struct CallAwaitingResponse;
  friend class QueueTransferTask;
  friend struct ResponseTransferCallbacks;

  struct CallAwaitingResponse {
    ~CallAwaitingResponse();

    void HandleTimeout(ev::timer &watcher, int revents);

    Connection *conn;
    std::shared_ptr<OutboundCall> call;
    ev::timer timeout_timer;

    double remaining_timeout;
  };

  typedef std::unordered_map<uint64_t, CallAwaitingResponse*> car_map_t;
  typedef std::unordered_map<uint64_t, InboundCall*> inbound_call_map_t;

  // Returns the next valid (positive) sequential call ID by incrementing a counter
  // and ensuring we roll over from INT32_MAX to 0.
  // Negative numbers are reserved for special purposes.
  int32_t GetNextCallId() {
    int32_t call_id = next_call_id_;
    if (PREDICT_FALSE(next_call_id_ == std::numeric_limits<int32_t>::max())) {
      next_call_id_ = 0;
    } else {
      next_call_id_++;
    }
    return call_id;
  }

  void HandleIncomingCall(gscoped_ptr<InboundTransfer> transfer);
  void HandleCallResponse(gscoped_ptr<InboundTransfer> transfer);
  void HandleOutboundCallTimeout(CallAwaitingResponse *car);
  void QueueOutbound(gscoped_ptr<OutboundTransfer> transfer);

  ReactorThread * const reactor_thread_;
  const Sockaddr remote_;
  std::unique_ptr<Socket> socket_;
  UserCredentials user_credentials_;
  Direction direction_;
  MonoTime last_activity_time_;
  gscoped_ptr<InboundTransfer> inbound_;
  ev::io write_io_;
  ev::io read_io_;

  bool is_epoll_registered_;

  boost::intrusive::list<OutboundTransfer> outbound_transfers_; // NOLINT(*)

  car_map_t awaiting_response_;

  inbound_call_map_t calls_being_handled_;

  int32_t next_call_id_;

  Status shutdown_status_;

  std::vector<Slice> slices_tmp_;

  ObjectPool<CallAwaitingResponse> car_pool_;
  typedef ObjectPool<CallAwaitingResponse>::scoped_ptr scoped_car;

  SaslClient sasl_client_;

  SaslServer sasl_server_;

  bool negotiation_complete_;
};

} // namespace rpc
} // namespace mprmpr

#endif

#include "mprmpr/rpc/acceptor_pool.h"

#include <gflags/gflags.h>
#include <glog/logging.h>
#include <inttypes.h>
#include <iostream>
#include <pthread.h>
#include <stdint.h>
#include <string>
#include <vector>

#include "mprmpr/base/ref_counted.h"
#include "mprmpr/base/strings/substitute.h"
#include "mprmpr/rpc/messenger.h"
//#include "mprmpr/util/flag_tags.h"
//#include "mprmpr/util/logging.h"
#include "mprmpr/util/metrics.h"
#include "mprmpr/util/net/sockaddr.h"
#include "mprmpr/util/net/socket.h"
#include "mprmpr/util/status.h"
#include "mprmpr/util/thread.h"

using google::protobuf::Message;
using std::string;

METRIC_DEFINE_counter(server, rpc_connections_accepted,
                      "RPC Connections Accepted",
                      mprmpr::MetricUnit::kConnections,
                      "Number of incoming TCP connections made to the RPC server");

DEFINE_int32(rpc_acceptor_listen_backlog, 128,
             "Socket backlog parameter used when listening for RPC connections. "
             "This defines the maximum length to which the queue of pending "
             "TCP connections inbound to the RPC server may grow. If a connection "
             "request arrives when the queue is full, the client may receive "
             "an error. Higher values may help the server ride over bursts of "
             "new inbound connection requests.");
//TAG_FLAG(rpc_acceptor_listen_backlog, advanced);

namespace mprmpr {
namespace rpc {

AcceptorPool::AcceptorPool(Messenger* messenger, Socket* socket,
                           Sockaddr bind_address)
    : messenger_(messenger),
      socket_(socket->Release()),
      bind_address_(std::move(bind_address)),
      rpc_connections_accepted_(METRIC_rpc_connections_accepted.Instantiate(
          messenger->metric_entity())),
      closing_(false) {}

AcceptorPool::~AcceptorPool() {
  Shutdown();
}

Status AcceptorPool::Start(int num_threads) {
  RETURN_NOT_OK(socket_.Listen(FLAGS_rpc_acceptor_listen_backlog));

  for (int i = 0; i < num_threads; i++) {
    scoped_refptr<mprmpr::Thread> new_thread;
    Status s = mprmpr::Thread::Create("acceptor pool", "acceptor",
        &AcceptorPool::RunThread, this, &new_thread);
    if (!s.ok()) {
      Shutdown();
      return s;
    }
    threads_.push_back(new_thread);
  }
  return Status::OK();
}

void AcceptorPool::Shutdown() {
  if (Acquire_CompareAndSwap(&closing_, false, true) != false) {
    VLOG(2) << "Acceptor Pool on " << bind_address_.ToString()
            << " already shut down";
    return;
  }

  WARN_NOT_OK(socket_.Shutdown(true, true),
              strings::Substitute("Could not shut down acceptor socket on $0",
                                  bind_address_.ToString()));

  for (const scoped_refptr<mprmpr::Thread>& thread : threads_) {
    CHECK_OK(ThreadJoiner(thread.get()).Join());
  }
  threads_.clear();
}

Sockaddr AcceptorPool::bind_address() const {
  return bind_address_;
}

Status AcceptorPool::GetBoundAddress(Sockaddr* addr) const {
  return socket_.GetSocketAddress(addr);
}

void AcceptorPool::RunThread() {
  while (true) {
    Socket new_sock;
    Sockaddr remote;
    VLOG(2) << "calling accept() on socket " << socket_.GetFd()
            << " listening on " << bind_address_.ToString();
    Status s = socket_.Accept(&new_sock, &remote, Socket::FLAG_NONBLOCKING);
    if (!s.ok()) {
      if (base::subtle::Release_Load(&closing_)) {
        break;
      }
//      KLOG_EVERY_N_SECS(WARNING, 1) << "AcceptorPool: accept failed: " << s.ToString()
//                                    << THROTTLE_MSG;
      continue;
    }
    s = new_sock.SetNoDelay(true);
    if (!s.ok()) {
//      KLOG_EVERY_N_SECS(WARNING, 1) << "Acceptor with remote = " << remote.ToString()
//          << " failed to set TCP_NODELAY on a newly accepted socket: "
//          << s.ToString() << THROTTLE_MSG;
      continue;
    }
    rpc_connections_accepted_->Increment();
    messenger_->RegisterInboundSocket(&new_sock, remote);
  }
  VLOG(1) << "AcceptorPool shutting down.";
}

} // namespace rpc
} // namespace kudu

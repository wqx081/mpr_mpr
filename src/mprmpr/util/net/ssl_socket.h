#ifndef KUDU_UTIL_NET_SSL_SOCKET_H
#define KUDU_UTIL_NET_SSL_SOCKET_H

#include <sys/uio.h>

#include <string>

#include "mprmpr/base/macros.h"
#include "mprmpr/util/net/socket.h"
#include "mprmpr/util/status.h"

struct ssl_st;
typedef ssl_st SSL;

namespace mprmpr {

class Sockaddr;

class SSLSocket : public Socket {
 public:
  SSLSocket(int fd, SSL* ssl, bool is_server);

  ~SSLSocket();

  // Do the SSL handshake as a client or a server and verify that the credentials were correctly
  // verified.
  Status DoHandshake();

  Status Write(const uint8_t *buf, int32_t amt, int32_t *nwritten) override;

  Status Writev(const struct ::iovec *iov, int iov_len, int32_t *nwritten) override;

  Status Recv(uint8_t *buf, int32_t amt, int32_t *nread) override;

  // Shutdown the connection and free the SSL state for this connection.
  Status Close() override;
 private:
  SSL* ssl_; // Owned.
  bool is_server_;
};

} // namespace mprmpr

#endif

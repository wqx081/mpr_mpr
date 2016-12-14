#ifndef KUDU_RPC_BLOCKING_OPS_H
#define KUDU_RPC_BLOCKING_OPS_H

#include <set>
#include <string>

namespace google {
namespace protobuf {
class MessageLite;
} // namespace protobuf
} // namespace google

namespace mprmpr {

class faststring;
class MonoTime;
class Slice;
class Sockaddr;
class Socket;
class Status;

namespace rpc {

class SaslMessagePB;

Status EnsureBlockingMode(const Socket* const sock);
Status SendFramedMessageBlocking(Socket* sock, const google::protobuf::MessageLite& header,
    const google::protobuf::MessageLite& msg, const MonoTime& deadline);
Status ReceiveFramedMessageBlocking(Socket* sock, faststring* recv_buf,
    google::protobuf::MessageLite* header, Slice* param_buf, const MonoTime& deadline);

} // namespace rpc
} // namespace mprmpr

#endif  // KUDU_RPC_BLOCKING_OPS_H

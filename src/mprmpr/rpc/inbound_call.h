#ifndef KUDU_RPC_INBOUND_CALL_H
#define KUDU_RPC_INBOUND_CALL_H

#include <glog/logging.h>
#include <string>
#include <vector>

#include "mprmpr/base/gscoped_ptr.h"
#include "mprmpr/base/stl_util.h"
#include "mprmpr/base/macros.h"
#include "mprmpr/base/ref_counted.h"
#include "mprmpr/rpc/remote_method.h"
#include "mprmpr/rpc/service_if.h"
#include "mprmpr/rpc/rpc_header.pb.h"
#include "mprmpr/rpc/transfer.h"
#include "mprmpr/util/faststring.h"
#include "mprmpr/util/monotime.h"
#include "mprmpr/util/slice.h"
#include "mprmpr/util/status.h"

namespace google {
namespace protobuf {
class Message;
} // namespace protobuf
} // namespace google

namespace mprmpr {

class Histogram;
class Trace;

namespace rpc {

class Connection;
class DumpRunningRpcsRequestPB;
class RpcCallInProgressPB;
struct RpcMethodInfo;
class RpcSidecar;
class UserCredentials;

struct InboundCallTiming {
  MonoTime time_received;   // Time the call was first accepted.
  MonoTime time_handled;    // Time the call handler was kicked off.
  MonoTime time_completed;  // Time the call handler completed.

  MonoDelta TotalDuration() const {
    return time_completed - time_received;
  }
};

// Inbound call on server
class InboundCall {
 public:
  explicit InboundCall(Connection* conn);
  ~InboundCall();

  Status ParseFrom(gscoped_ptr<InboundTransfer> transfer);

  const Slice &serialized_request() const {
    return serialized_request_;
  }

  const RemoteMethod& remote_method() const {
    return remote_method_;
  }

  const int32_t call_id() const {
    return header_.call_id();
  }

  void RespondSuccess(const google::protobuf::MessageLite& response);

  void RespondFailure(ErrorStatusPB::RpcErrorCodePB error_code,
                      const Status &status);

  void RespondUnsupportedFeature(const std::vector<uint32_t>& unsupported_features);

  void RespondApplicationError(int error_ext_id, const std::string& message,
                               const google::protobuf::MessageLite& app_error_pb);

  static void ApplicationErrorToPB(int error_ext_id, const std::string& message,
                                   const google::protobuf::MessageLite& app_error_pb,
                                   ErrorStatusPB* err);

  void SerializeResponseTo(std::vector<Slice>* slices) const;

  // See RpcContext::AddRpcSidecar()
  Status AddRpcSidecar(gscoped_ptr<RpcSidecar> car, int* idx);

  std::string ToString() const;

  void DumpPB(const DumpRunningRpcsRequestPB& req, RpcCallInProgressPB* resp);

  const UserCredentials& user_credentials() const;

  const Sockaddr& remote_address() const;

  const scoped_refptr<Connection>& connection() const;

  Trace* trace();

  const InboundCallTiming& timing() const {
    return timing_;
  }

  const RequestHeader& header() const {
    return header_;
  }

  void set_method_info(scoped_refptr<RpcMethodInfo> info) {
    method_info_ = std::move(info);
  }

  RpcMethodInfo* method_info() {
    return method_info_.get();
  }

  void RecordCallReceived();

  void RecordHandlingStarted(scoped_refptr<Histogram> incoming_queue_time);

  bool ClientTimedOut() const;

  MonoTime GetClientDeadline() const;

  // Return the time when this call was received.
  MonoTime GetTimeReceived() const;

  // Returns the set of application-specific feature flags required to service
  // the RPC.
  std::vector<uint32_t> GetRequiredFeatures() const;

 private:
  friend class RpczStore;

  // Serialize and queue the response.
  void Respond(const google::protobuf::MessageLite& response,
               bool is_success);

  // Serialize a response message for either success or failure. If it is a success,
  // 'response' should be the user-defined response type for the call. If it is a
  // failure, 'response' should be an ErrorStatusPB instance.
  void SerializeResponseBuffer(const google::protobuf::MessageLite& response,
                               bool is_success);

  // When RPC call Handle() completed execution on the server side.
  // Updates the Histogram with time elapsed since the call was started,
  // and should only be called once on a given instance.
  // Not thread-safe. Should only be called by the current "owner" thread.
  void RecordHandlingCompleted();

  // The connection on which this inbound call arrived.
  scoped_refptr<Connection> conn_;

  // The header of the incoming call. Set by ParseFrom()
  RequestHeader header_;

  // The serialized bytes of the request param protobuf. Set by ParseFrom().
  // This references memory held by 'transfer_'.
  Slice serialized_request_;

  // The transfer that produced the call.
  // This is kept around because it retains the memory referred to
  // by 'serialized_request_' above.
  gscoped_ptr<InboundTransfer> transfer_;

  // The buffers for serialized response. Set by SerializeResponseBuffer().
  faststring response_hdr_buf_;
  faststring response_msg_buf_;

  // Vector of additional sidecars that are tacked on to the call's response
  // after serialization of the protobuf. See rpc/rpc_sidecar.h for more info.
  std::vector<RpcSidecar*> sidecars_;
  ElementDeleter sidecars_deleter_;

  // The trace buffer.
  scoped_refptr<Trace> trace_;

  // Timing information related to this RPC call.
  InboundCallTiming timing_;

  // Proto service this calls belongs to. Used for routing.
  // This field is filled in when the inbound request header is parsed.
  RemoteMethod remote_method_;

  // After the method has been looked up within the service, this is filled in
  // to point to the information about this method. Acts as a pointer back to
  // per-method info such as tracing.
  scoped_refptr<RpcMethodInfo> method_info_;

  DISALLOW_COPY_AND_ASSIGN(InboundCall);
};

} // namespace rpc
} // namespace mprmpr

#endif

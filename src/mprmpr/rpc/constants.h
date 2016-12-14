#ifndef KUDU_RPC_RPC_CONSTANTS_H
#define KUDU_RPC_RPC_CONSTANTS_H

#include <cstdint>
#include <set>

#include "mprmpr/rpc/rpc_header.pb.h"

namespace mprmpr {
namespace rpc {

// Magic number bytes sent at connection setup time.
extern const char* const kMagicNumber;

// App name for SASL library init
extern const char* const kSaslAppName;

// Network protocol name for SASL library init
extern const char* const kSaslProtoName;

// Current version of the RPC protocol.
static const uint32_t kCurrentRpcVersion = 9;

// From Hadoop.
static const int32_t kInvalidCallId = -2;
static const int32_t kConnectionContextCallId = -3;
static const int32_t kSaslCallId = -33;

static const uint8_t kMagicNumberLength = 4;
static const uint8_t kHeaderFlagsLength = 3;

// There is a 4-byte length prefix before any packet.
static const uint8_t kMsgLengthPrefixLength = 4;

// The set of RPC features that this server build supports.
// Non-const for testing.
extern std::set<RpcFeatureFlag> kSupportedServerRpcFeatureFlags;

// The set of RPC features that this client build supports.
// Non-const for testing.
extern std::set<RpcFeatureFlag> kSupportedClientRpcFeatureFlags;

} // namespace rpc
} // namespace mprmpr

#endif // KUDU_RPC_RPC_CONSTANTS_H

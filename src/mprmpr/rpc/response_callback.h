#ifndef KUDU_RPC_RESPONSE_CALLBACK_H
#define KUDU_RPC_RESPONSE_CALLBACK_H

#include <functional>

namespace mprmpr {
namespace rpc {

typedef std::function<void()> ResponseCallback;

}
}

#endif

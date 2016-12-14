#ifndef MPRMPR_MASTER_MASTER_H_
#define MPRMPR_MASTER_MASTER_H_

#include <atomic>
#include <memory>
#include <string>
#include <vector>

#include "mprmpr/common/common.h"
#include "mprmpr/base/gscoped_ptr.h"
#include "mprmpr/base/macros.h"
#include "mprmpr/master/master_options.h"
#include "mprmpr/master/master.pb.h"
#include "mprmpr/server/server_base.h"
#include "mprmpr/util/metrics.h"
#include "mprmpr/util/promise.h"
#include "mprmpr/util/status.h"

namespace mprmpr {

class RpcServer;
struct RpcServerOptions;
class ThreadPool;


namespace rpc {
class Messenger;
class ServicePool;
} // namespace rpc

namespace master {

class WorkerManager;
class MasterPathHandlers;

class Master : public server::ServerBase {

};

} // namespace master
} // namespace mprmpr
#endif // MPRMPR_MASTER_MASTER_H_

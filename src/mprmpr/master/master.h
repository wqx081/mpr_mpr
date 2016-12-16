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
 public:
  static const uint16_t kDefaultPort = 9982;
  static const uint16_t kDefaultWebPort = 9981;

  explicit Master(const MasterOptions& opts);
  ~Master();

  Status Init();
  Status Start();

  Status StartAsync();

  void Shutdown();
  std::string ToString() const;
  WorkerManager* worker_manager() { return worker_manager_.get(); }

  const MasterOptions& options();
  Status GetMasterRegistration(ServerRegistrationPB* registration) const;
  bool IsShutdown() const { return state_ == kStopped; }

 private:
  friend class MasterTest;

  Status InitMasterRegistration();

  enum MasterState {
    kStopped,
    kInitialized,
    kRunning
  };

  MasterState state_;

  gscoped_ptr<WorkerManager> worker_manager_;

  MasterOptions options_;

  ServerRegistrationPB registration_;
  std::atomic<bool> registration_initialized_;

  DISALLOW_COPY_AND_ASSIGN(Master);
};

} // namespace master
} // namespace mprmpr
#endif // MPRMPR_MASTER_MASTER_H_

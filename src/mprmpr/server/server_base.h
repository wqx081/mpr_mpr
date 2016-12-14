#ifndef ANT_SERVER_SERVER_BASE_H_
#define ANT_SERVER_SERVER_BASE_H_

#include <memory>
#include <string>

#include "mprmpr/base/gscoped_ptr.h"
#include "mprmpr/base/macros.h"
#include "mprmpr/base/ref_counted.h"
#include "mprmpr/rpc/service_if.h"
#include "mprmpr/server/server_base_options.h"
#include "mprmpr/util/status.h"

namespace mprmpr {

class Env;
class MemTracker;
class MetricEntity;
class MetricRegistry;
class NodeInstancePB;
class RpcServer;
class Sockaddr;
class Thread;
class WebServer;

class ScopedGLogMetrics;

namespace rpc {
class Messenger;
class ServiceIf;
} // namespace rpc

namespace server {

class Clock;
struct ServerBaseOptions;
class ServerStatusPB;

class ServerBase {
 public:
  const RpcServer* rpc_server() const { return rpc_server_.get(); }
  const WebServer* web_server() const { return web_server_.get(); }
  const std::shared_ptr<rpc::Messenger>& messenger() const {
    return messenger_;
  }

  Sockaddr first_rpc_address() const;
  Sockaddr first_http_address() const;

  const NodeInstancePB& instance_pb() const;

  const std::shared_ptr<MemTracker>& mem_tracker() const {
    return mem_tracker_;
  }
  const scoped_refptr<MetricEntity>& metric_entity() const {
    return metric_entity_;
  }

  MetricRegistry* metric_registry() {
    return metric_registry_.get();
  }

  const scoped_refptr<rpc::ResultTracker>& result_tracker() const {
    return result_tracker_;
  }

  Clock* clock() {
    return clock_.get();
  }

  void GetStatusPB(ServerStatusPB* status) const;

 protected:
  ServerBase(std::string name,
             const ServerBaseOptions& options,
	     const std::string& metrics_namespace);
  virtual ~ServerBase();

  Status Init();
  Status RegisterService(gscoped_ptr<rpc::ServiceIf> rpc_impl);
  Status Start();
  void Shutdown();

  const std::string name_;

  std::shared_ptr<MemTracker> mem_tracker_;
  gscoped_ptr<MetricRegistry> metric_registry_;
  scoped_refptr<MetricEntity> metric_entity_;
  gscoped_ptr<RpcServer> rpc_server_;
  gscoped_ptr<WebServer> web_server_;
  std::shared_ptr<rpc::Messenger> messenger_;
  scoped_refptr<rpc::ResultTracker> result_tracker_;
  bool is_first_run_;

  scoped_refptr<Clock> clock_;

  gscoped_ptr<NodeInstancePB> instance_pb_;

 private:
  void GenerateInstanceID();
  Status DumpServerInfo(const std::string& path,
		        const std::string& format) const;
  Status StartMetricsLogging();
  void MetricsLoggingThread();
  std::string FooterHtml() const;

  ServerBaseOptions options_;

  scoped_refptr<Thread> metrics_logging_thread_;
  CountDownLatch stop_metrics_logging_latch_;

  gscoped_ptr<ScopedGLogMetrics> glog_metrics_;
  
  DISALLOW_COPY_AND_ASSIGN(ServerBase);
};

} // namespace server
} // namespace mprmpr
#endif // ANT_SERVER_SERVER_BASE_H_

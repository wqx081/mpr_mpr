#ifndef KUDU_SERVICE_POOL_H
#define KUDU_SERVICE_POOL_H

#include <string>
#include <vector>

#include "mprmpr/base/macros.h"
#include "mprmpr/base/gscoped_ptr.h"
#include "mprmpr/base/ref_counted.h"
#include "mprmpr/rpc/rpc_service.h"
#include "mprmpr/rpc/service_queue.h"
#include "mprmpr/util/mutex.h"
#include "mprmpr/util/thread.h"
#include "mprmpr/util/status.h"

namespace mprmpr {

class Counter;
class Histogram;
class MetricEntity;
class Socket;

namespace rpc {

class Messenger;
class ServiceIf;

// A pool of threads that handle new incoming RPC calls.
// Also includes a queue that calls get pushed onto for handling by the pool.
class ServicePool : public RpcService {
 public:
  ServicePool(gscoped_ptr<ServiceIf> service,
              const scoped_refptr<MetricEntity>& metric_entity,
              size_t service_queue_length);
  virtual ~ServicePool();

  // Start up the thread pool.
  virtual Status Init(int num_threads);

  // Shut down the queue and the thread pool.
  virtual void Shutdown();

  RpcMethodInfo* LookupMethod(const RemoteMethod& method) override;

  virtual Status QueueInboundCall(gscoped_ptr<InboundCall> call) OVERRIDE;

  const Counter* RpcsTimedOutInQueueMetricForTests() const {
    return rpcs_timed_out_in_queue_.get();
  }

  const Histogram* IncomingQueueTimeMetricForTests() const {
    return incoming_queue_time_.get();
  }

  const Counter* RpcsQueueOverflowMetric() const {
    return rpcs_queue_overflow_.get();
  }

  const std::string service_name() const;

 private:
  void RunThread();
  void RejectTooBusy(InboundCall* c);

  gscoped_ptr<ServiceIf> service_;
  std::vector<scoped_refptr<mprmpr::Thread> > threads_;
  LifoServiceQueue service_queue_;
  scoped_refptr<Histogram> incoming_queue_time_;
  scoped_refptr<Counter> rpcs_timed_out_in_queue_;
  scoped_refptr<Counter> rpcs_queue_overflow_;

  mutable Mutex shutdown_lock_;
  bool closing_;

  DISALLOW_COPY_AND_ASSIGN(ServicePool);
};

} // namespace rpc
} // namespace mprmpr

#endif

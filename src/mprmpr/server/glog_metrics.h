#ifndef KUDU_SERVER_GLOG_METRICS_H
#define KUDU_SERVER_GLOG_METRICS_H

#include "mprmpr/base/macros.h"
#include "mprmpr/base/gscoped_ptr.h"
#include "mprmpr/base/ref_counted.h"

namespace google {
class LogSink;
} // namespace google

namespace mprmpr {
class MetricEntity;

// Attaches GLog metrics to the given entity, for the duration of this
// scoped object's lifetime.
//
// NOTE: the metrics are collected process-wide, not confined to any set of
// threads, etc.
class ScopedGLogMetrics {
 public:
  explicit ScopedGLogMetrics(const scoped_refptr<MetricEntity>& entity);
  ~ScopedGLogMetrics();

 private:
  gscoped_ptr<google::LogSink> sink_;
};


// Registers glog-related metrics.
// This can be called multiple times on different entities, though the resulting
// metrics will be identical, since the GLog tracking is process-wide.
void RegisterGLogMetrics(const scoped_refptr<MetricEntity>& entity);

} // namespace mprmpr
#endif /* KUDU_SERVER_GLOG_METRICS_H */

#ifndef KUDU_UTIL_METRICS_H
#define KUDU_UTIL_METRICS_H

#include <algorithm>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include <gtest/gtest_prod.h>

#include "mprmpr/base/bind.h"
#include "mprmpr/base/callback.h"
#include "mprmpr/base/casts.h"
#include "mprmpr/base/map-util.h"
#include "mprmpr/base/ref_counted.h"
#include "mprmpr/base/singleton.h"

#include "mprmpr/util/atomic.h"
#include "mprmpr/util/jsonwriter.h"
#include "mprmpr/util/locks.h"
#include "mprmpr/util/monotime.h"
#include "mprmpr/util/status.h"
#include "mprmpr/util/striped64.h"


// Define a new entity type.
//
// The metrics subsystem itself defines the entity type 'server', but other
// entity types can be registered using this macro.
#define METRIC_DEFINE_entity(name)                               \
  ::mprmpr::MetricEntityPrototype METRIC_ENTITY_##name(#name)

// Convenience macros to define metric prototypes.
// See the documentation at the top of this file for example usage.
#define METRIC_DEFINE_counter(entity, name, label, unit, desc)   \
  ::mprmpr::CounterPrototype METRIC_##name(                        \
      ::mprmpr::MetricPrototype::CtorArgs(#entity, #name, label, unit, desc))

#define METRIC_DEFINE_gauge_string(entity, name, label, unit, desc, ...) \
  ::mprmpr::GaugePrototype<std::string> METRIC_##name(                 \
      ::mprmpr::MetricPrototype::CtorArgs(#entity, #name, label, unit, desc, ## __VA_ARGS__))
#define METRIC_DEFINE_gauge_bool(entity, name, label, unit, desc, ...) \
  ::mprmpr::GaugePrototype<bool> METRIC_##  name(                    \
      ::mprmpr::MetricPrototype::CtorArgs(#entity, #name, label, unit, desc, ## __VA_ARGS__))
#define METRIC_DEFINE_gauge_int32(entity, name, label, unit, desc, ...) \
  ::mprmpr::GaugePrototype<int32_t> METRIC_##name(                   \
      ::mprmpr::MetricPrototype::CtorArgs(#entity, #name, label, unit, desc, ## __VA_ARGS__))
#define METRIC_DEFINE_gauge_uint32(entity, name, label, unit, desc, ...) \
  ::mprmpr::GaugePrototype<uint32_t> METRIC_##name(                    \
      ::mprmpr::MetricPrototype::CtorArgs(#entity, #name, label, unit, desc, ## __VA_ARGS__))
#define METRIC_DEFINE_gauge_int64(entity, name, label, unit, desc, ...) \
  ::mprmpr::GaugePrototype<int64_t> METRIC_##name(                   \
      ::mprmpr::MetricPrototype::CtorArgs(#entity, #name, label, unit, desc, ## __VA_ARGS__))
#define METRIC_DEFINE_gauge_uint64(entity, name, label, unit, desc, ...) \
  ::mprmpr::GaugePrototype<uint64_t> METRIC_##name(                    \
      ::mprmpr::MetricPrototype::CtorArgs(#entity, #name, label, unit, desc, ## __VA_ARGS__))
#define METRIC_DEFINE_gauge_double(entity, name, label, unit, desc, ...) \
  ::mprmpr::GaugePrototype<double> METRIC_##name(                      \
      ::mprmpr::MetricPrototype::CtorArgs(#entity, #name, label, unit, desc, ## __VA_ARGS__))

#define METRIC_DEFINE_histogram(entity, name, label, unit, desc, max_val, num_sig_digits) \
  ::mprmpr::HistogramPrototype METRIC_##name(                                       \
      ::mprmpr::MetricPrototype::CtorArgs(#entity, #name, label, unit, desc), \
    max_val, num_sig_digits)

// The following macros act as forward declarations for entity types and metric prototypes.
#define METRIC_DECLARE_entity(name) \
  extern ::mprmpr::MetricEntityPrototype METRIC_ENTITY_##name
#define METRIC_DECLARE_counter(name)                             \
  extern ::mprmpr::CounterPrototype METRIC_##name
#define METRIC_DECLARE_gauge_string(name) \
  extern ::mprmpr::GaugePrototype<std::string> METRIC_##name
#define METRIC_DECLARE_gauge_bool(name) \
  extern ::mprmpr::GaugePrototype<bool> METRIC_##name
#define METRIC_DECLARE_gauge_int32(name) \
  extern ::mprmpr::GaugePrototype<int32_t> METRIC_##name
#define METRIC_DECLARE_gauge_uint32(name) \
  extern ::mprmpr::GaugePrototype<uint32_t> METRIC_##name
#define METRIC_DECLARE_gauge_int64(name) \
  extern ::mprmpr::GaugePrototype<int64_t> METRIC_##name
#define METRIC_DECLARE_gauge_uint64(name) \
  extern ::mprmpr::GaugePrototype<uint64_t> METRIC_##name
#define METRIC_DECLARE_gauge_double(name) \
  extern ::mprmpr::GaugePrototype<double> METRIC_##name
#define METRIC_DECLARE_histogram(name) \
  extern ::mprmpr::HistogramPrototype METRIC_##name

#define METRIC_DEFINE_gauge_size METRIC_DEFINE_gauge_uint64
#define METRIC_DECLARE_gauge_size METRIC_DECLARE_gauge_uint64

namespace mprmpr {

class Counter;
class CounterPrototype;

template<typename T>
class AtomicGauge;
template<typename T>
class FunctionGauge;
class Gauge;
template<typename T>
class GaugePrototype;

class Metric;
class MetricEntityPrototype;
class MetricPrototype;
class MetricRegistry;

class HdrHistogram;
class Histogram;
class HistogramPrototype;
class HistogramSnapshotPB;

class MetricEntity;

} // namespace mprmpr

// Forward-declare the generic 'server' entity type.
// We have to do this here below the forward declarations, but not
// in the mprmpr namespace.
METRIC_DECLARE_entity(server);

namespace mprmpr {

// Unit types to be used with metrics.
// As additional units are required, add them to this enum and also to Name().
struct MetricUnit {
  enum Type {
    kCacheHits,
    kCacheQueries,
    kBytes,
    kRequests,
    kEntries,
    kRows,
    kCells,
    kConnections,
    kOperations,
    kProbes,
    kNanoseconds,
    kMicroseconds,
    kMilliseconds,
    kSeconds,
    kThreads,
    kTransactions,
    kUnits,
    kScanners,
    kMaintenanceOperations,
    kBlocks,
    kLogBlockContainers,
    kTasks,
    kMessages,
    kContextSwitches,
    kDataDirectories,
  };
  static const char* Name(Type unit);
};

class MetricType {
 public:
  enum Type { kGauge, kCounter, kHistogram };
  static const char* Name(Type t);
 private:
  static const char* const kGaugeType;
  static const char* const kCounterType;
  static const char* const kHistogramType;
};

struct MetricJsonOptions {
  MetricJsonOptions() :
    include_raw_histograms(false),
    include_schema_info(false) {
  }

  // Include the raw histogram values and counts in the JSON output.
  // This allows consumers to do cross-server aggregation or window
  // data over time.
  // Default: false
  bool include_raw_histograms;

  // Include the metrics "schema" information (i.e description, label,
  // unit, etc).
  // Default: false
  bool include_schema_info;
};

class MetricEntityPrototype {
 public:
  explicit MetricEntityPrototype(const char* name);
  ~MetricEntityPrototype();

  const char* name() const { return name_; }

  // Find or create an entity with the given ID within the provided 'registry'.
  scoped_refptr<MetricEntity> Instantiate(
      MetricRegistry* registry,
      const std::string& id) const {
    return Instantiate(registry, id, std::unordered_map<std::string, std::string>());
  }

  // If the entity already exists, then 'initial_attrs' will replace all existing
  // attributes.
  scoped_refptr<MetricEntity> Instantiate(
      MetricRegistry* registry,
      const std::string& id,
      const std::unordered_map<std::string, std::string>& initial_attrs) const;

 private:
  const char* const name_;

  DISALLOW_COPY_AND_ASSIGN(MetricEntityPrototype);
};

class MetricEntity : public base::RefCountedThreadSafe<MetricEntity> {
 public:
  typedef std::unordered_map<const MetricPrototype*, scoped_refptr<Metric> > MetricMap;
  typedef std::unordered_map<std::string, std::string> AttributeMap;

  scoped_refptr<Counter> FindOrCreateCounter(const CounterPrototype* proto);
  scoped_refptr<Histogram> FindOrCreateHistogram(const HistogramPrototype* proto);

  template<typename T>
  scoped_refptr<AtomicGauge<T> > FindOrCreateGauge(const GaugePrototype<T>* proto,
                                                   const T& initial_value);

  template<typename T>
  scoped_refptr<FunctionGauge<T> > FindOrCreateFunctionGauge(const GaugePrototype<T>* proto,
                                                             const base::Callback<T()>& function);

  // Return the metric instantiated from the given prototype, or NULL if none has been
  // instantiated. Primarily used by tests trying to read metric values.
  scoped_refptr<Metric> FindOrNull(const MetricPrototype& prototype) const;

  const std::string& id() const { return id_; }

  // See MetricRegistry::WriteAsJson()
  Status WriteAsJson(JsonWriter* writer,
                     const std::vector<std::string>& requested_metrics,
                     const MetricJsonOptions& opts) const;

  const MetricMap& UnsafeMetricsMapForTests() const { return metric_map_; }

  // Mark that the given metric should never be retired until the metric
  // registry itself destructs. This is useful for system metrics such as
  // tcmalloc, etc, which should live as long as the process itself.
  void NeverRetire(const scoped_refptr<Metric>& metric);

  // Scan the metrics map for metrics needing retirement, removing them as necessary.
  //
  // Metrics are retired when they are no longer referenced outside of the metrics system
  // itself. Additionally, we only retire a metric that has been in this state for
  // at least FLAGS_metrics_retirement_age_ms milliseconds.
  void RetireOldMetrics();

  // Replaces all attributes for this entity.
  // Any attributes currently set, but not in 'attrs', are removed.
  void SetAttributes(const AttributeMap& attrs);

  // Set a particular attribute. Replaces any current value.
  void SetAttribute(const std::string& key, const std::string& val);

  int num_metrics() const {
    std::lock_guard<simple_spinlock> l(lock_);
    return metric_map_.size();
  }

 private:
  friend class MetricRegistry;
  friend class base::RefCountedThreadSafe<MetricEntity>;

  MetricEntity(const MetricEntityPrototype* prototype, std::string id,
               AttributeMap attributes);
  ~MetricEntity();

  // Ensure that the given metric prototype is allowed to be instantiated
  // within this entity. This entity's type must match the expected entity
  // type defined within the metric prototype.
  void CheckInstantiation(const MetricPrototype* proto) const;

  const MetricEntityPrototype* const prototype_;
  const std::string id_;

  mutable simple_spinlock lock_;

  // Map from metric name to Metric object. Protected by lock_.
  MetricMap metric_map_;

  // The key/value attributes. Protected by lock_
  AttributeMap attributes_;

  // The set of metrics which should never be retired. Protected by lock_.
  std::vector<scoped_refptr<Metric> > never_retire_metrics_;
};

// Base class to allow for putting all metrics into a single container.
// See documentation at the top of this file for information on metrics ownership.
class Metric : public base::RefCountedThreadSafe<Metric> {
 public:
  // All metrics must be able to render themselves as JSON.
  virtual Status WriteAsJson(JsonWriter* writer,
                             const MetricJsonOptions& opts) const = 0;

  const MetricPrototype* prototype() const { return prototype_; }

 protected:
  explicit Metric(const MetricPrototype* prototype);
  virtual ~Metric();

  const MetricPrototype* const prototype_;

 private:
  friend class MetricEntity;
  friend class base::RefCountedThreadSafe<Metric>;

  // The time at which we should retire this metric if it is still un-referenced outside
  // of the metrics subsystem. If this metric is not due for retirement, this member is
  // uninitialized.
  MonoTime retire_time_;

  DISALLOW_COPY_AND_ASSIGN(Metric);
};

// Registry of all the metrics for a server.
//
// This aggregates the MetricEntity objects associated with the server.
class MetricRegistry {
 public:
  MetricRegistry();
  ~MetricRegistry();

  scoped_refptr<MetricEntity> FindOrCreateEntity(const MetricEntityPrototype* prototype,
                                                 const std::string& id,
                                                 const MetricEntity::AttributeMap& initial_attrs);

  // Writes metrics in this registry to 'writer'.
  //
  // 'requested_metrics' is a set of substrings to match metric names against,
  // where '*' matches all metrics.
  //
  // The string matching can either match an entity ID or a metric name.
  // If it matches an entity ID, then all metrics for that entity will be printed.
  //
  // See the MetricJsonOptions struct definition above for options changing the
  // output of this function.
  Status WriteAsJson(JsonWriter* writer,
                     const std::vector<std::string>& requested_metrics,
                     const MetricJsonOptions& opts) const;

  // For each registered entity, retires orphaned metrics. If an entity has no more
  // metrics and there are no external references, entities are removed as well.
  //
  // See MetricEntity::RetireOldMetrics().
  void RetireOldMetrics();

  // Return the number of entities in this registry.
  int num_entities() const {
    std::lock_guard<simple_spinlock> l(lock_);
    return entities_.size();
  }

 private:
  typedef std::unordered_map<std::string, scoped_refptr<MetricEntity> > EntityMap;
  EntityMap entities_;

  mutable simple_spinlock lock_;
  DISALLOW_COPY_AND_ASSIGN(MetricRegistry);
};

// Registry of all of the metric and entity prototypes that have been
// defined.
//
// Prototypes are typically defined as static variables in different compilation
// units, and their constructors register themselves here. The registry is then
// used in order to dump metrics metadata to generate a Cloudera Manager MDL
// file.
//
// This class is thread-safe.
class MetricPrototypeRegistry {
 public:
  // Get the singleton instance.
  static MetricPrototypeRegistry* get();

  // Dump a JSON document including all of the registered entity and metric
  // prototypes.
  void WriteAsJson(JsonWriter* writer) const;

  // Convenience wrapper around WriteAsJson(...). This dumps the JSON information
  // to stdout and then exits.
  void WriteAsJsonAndExit() const;
 private:
  friend class Singleton<MetricPrototypeRegistry>;
  friend class MetricPrototype;
  friend class MetricEntityPrototype;
  MetricPrototypeRegistry() {}
  ~MetricPrototypeRegistry() {}

  // Register a metric prototype in the registry.
  void AddMetric(const MetricPrototype* prototype);

  // Register a metric entity prototype in the registry.
  void AddEntity(const MetricEntityPrototype* prototype);

  mutable simple_spinlock lock_;
  std::vector<const MetricPrototype*> metrics_;
  std::vector<const MetricEntityPrototype*> entities_;

  DISALLOW_COPY_AND_ASSIGN(MetricPrototypeRegistry);
};

enum PrototypeFlags {
  // Flag which causes a Gauge prototype to expose itself as if it
  // were a counter.
  EXPOSE_AS_COUNTER = 1 << 0
};

class MetricPrototype {
 public:
  // Simple struct to aggregate the arguments common to all prototypes.
  // This makes constructor chaining a little less tedious.
  struct CtorArgs {
    CtorArgs(const char* entity_type,
             const char* name,
             const char* label,
             MetricUnit::Type unit,
             const char* description,
             uint32_t flags = 0)
      : entity_type_(entity_type),
        name_(name),
        label_(label),
        unit_(unit),
        description_(description),
        flags_(flags) {
    }

    const char* const entity_type_;
    const char* const name_;
    const char* const label_;
    const MetricUnit::Type unit_;
    const char* const description_;
    const uint32_t flags_;
  };

  const char* entity_type() const { return args_.entity_type_; }
  const char* name() const { return args_.name_; }
  const char* label() const { return args_.label_; }
  MetricUnit::Type unit() const { return args_.unit_; }
  const char* description() const { return args_.description_; }
  virtual MetricType::Type type() const = 0;

  // Writes the fields of this prototype to the given JSON writer.
  void WriteFields(JsonWriter* writer,
                   const MetricJsonOptions& opts) const;

 protected:
  explicit MetricPrototype(CtorArgs args);
  virtual ~MetricPrototype() {
  }

  const CtorArgs args_;

 private:
  DISALLOW_COPY_AND_ASSIGN(MetricPrototype);
};

// A description of a Gauge.
template<typename T>
class GaugePrototype : public MetricPrototype {
 public:
  explicit GaugePrototype(const MetricPrototype::CtorArgs& args)
    : MetricPrototype(args) {
  }

  // Instantiate a "manual" gauge.
  scoped_refptr<AtomicGauge<T> > Instantiate(
      const scoped_refptr<MetricEntity>& entity,
      const T& initial_value) const {
    return entity->FindOrCreateGauge(this, initial_value);
  }

  // Instantiate a gauge that is backed by the given callback.
  scoped_refptr<FunctionGauge<T> > InstantiateFunctionGauge(
      const scoped_refptr<MetricEntity>& entity,
      const base::Callback<T()>& function) const {
    return entity->FindOrCreateFunctionGauge(this, function);
  }

  virtual MetricType::Type type() const OVERRIDE {
    if (args_.flags_ & EXPOSE_AS_COUNTER) {
      return MetricType::kCounter;
    } else {
      return MetricType::kGauge;
    }
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(GaugePrototype);
};

// Abstract base class to provide point-in-time metric values.
class Gauge : public Metric {
 public:
  explicit Gauge(const MetricPrototype* prototype)
    : Metric(prototype) {
  }
  virtual ~Gauge() {}
  virtual Status WriteAsJson(JsonWriter* w,
                             const MetricJsonOptions& opts) const OVERRIDE;
 protected:
  virtual void WriteValue(JsonWriter* writer) const = 0;
 private:
  DISALLOW_COPY_AND_ASSIGN(Gauge);
};

// Gauge implementation for string that uses locks to ensure thread safety.
class StringGauge : public Gauge {
 public:
  StringGauge(const GaugePrototype<std::string>* proto,
              std::string initial_value);
  std::string value() const;
  void set_value(const std::string& value);
 protected:
  virtual void WriteValue(JsonWriter* writer) const OVERRIDE;
 private:
  std::string value_;
  mutable simple_spinlock lock_;  // Guards value_
  DISALLOW_COPY_AND_ASSIGN(StringGauge);
};

// Lock-free implementation for types that are convertible to/from int64_t.
template <typename T>
class AtomicGauge : public Gauge {
 public:
  AtomicGauge(const GaugePrototype<T>* proto, T initial_value)
    : Gauge(proto),
      value_(initial_value) {
  }
  T value() const {
    return static_cast<T>(value_.Load(kMemOrderRelease));
  }
  virtual void set_value(const T& value) {
    value_.Store(static_cast<int64_t>(value), kMemOrderNoBarrier);
  }
  void Increment() {
    value_.IncrementBy(1, kMemOrderNoBarrier);
  }
  virtual void IncrementBy(int64_t amount) {
    value_.IncrementBy(amount, kMemOrderNoBarrier);
  }
  void Decrement() {
    IncrementBy(-1);
  }
  void DecrementBy(int64_t amount) {
    IncrementBy(-amount);
  }

 protected:
  virtual void WriteValue(JsonWriter* writer) const OVERRIDE {
    writer->Value(value());
  }
  AtomicInt<int64_t> value_;
 private:
  DISALLOW_COPY_AND_ASSIGN(AtomicGauge);
};

// Utility class to automatically detach FunctionGauges when a class destructs.
//
// Because FunctionGauges typically access class instance state, it's important to ensure
// that they are detached before the class destructs. One approach is to make all
// FunctionGauge instances be members of the class, and then call gauge_->Detach() in your
// class's destructor. However, it's easy to forget to do this, which would lead to
// heap-use-after-free bugs. This type of bug is easy to miss in unit tests because the
// tests don't always poll metrics. Using a FunctionGaugeDetacher member instead makes
// the detaching automatic and thus less error-prone.
//
// Example usage:
//
// METRIC_define_gauge_int64(my_metric, MetricUnit::kOperations, "My metric docs");
// class MyClassWithMetrics {
//  public:
//   MyClassWithMetrics(const scoped_refptr<MetricEntity>& entity) {
//     METRIC_my_metric.InstantiateFunctionGauge(entity,
//       base::Bind(&MyClassWithMetrics::ComputeMyMetric, Unretained(this)))
//       ->AutoDetach(&metric_detacher_);
//   }
//   ~MyClassWithMetrics() {
//   }
//
//   private:
//    int64_t ComputeMyMetric() {
//      // Compute some metric based on instance state.
//    }
//    FunctionGaugeDetacher metric_detacher_;
// };
class FunctionGaugeDetacher {
 public:
  FunctionGaugeDetacher();
  ~FunctionGaugeDetacher();

 private:
  template<typename T>
  friend class FunctionGauge;

  void OnDestructor(const base::Closure& c) {
    callbacks_.push_back(c);
  }

  std::vector<base::Closure> callbacks_;

  DISALLOW_COPY_AND_ASSIGN(FunctionGaugeDetacher);
};


// A Gauge that calls back to a function to get its value.
//
// This metric type should be used in cases where it is difficult to keep a running
// measure of a metric, but instead would like to compute the metric value whenever it is
// requested by a user.
//
// The lifecycle should be carefully considered when using a FunctionGauge. In particular,
// the bound function needs to always be safe to run -- so if it references a particular
// non-singleton class instance, the instance must out-live the function. Typically,
// the easiest way to ensure this is to use a FunctionGaugeDetacher (see above).
template <typename T>
class FunctionGauge : public Gauge {
 public:
  T value() const {
    std::lock_guard<simple_spinlock> l(lock_);
    return function_.Run();
  }

  virtual void WriteValue(JsonWriter* writer) const OVERRIDE {
    writer->Value(value());
  }

  // Reset this FunctionGauge to return a specific value.
  // This should be used during destruction. If you want a settable
  // Gauge, use a normal Gauge instead of a FunctionGauge.
  void DetachToConstant(T v) {
    std::lock_guard<simple_spinlock> l(lock_);
    function_ = base::Bind(&FunctionGauge::Return, v);
  }

  // Get the current value of the gauge, and detach so that it continues to return this
  // value in perpetuity.
  void DetachToCurrentValue() {
    T last_value = value();
    DetachToConstant(last_value);
  }

  // Automatically detach this gauge when the given 'detacher' destructs.
  // After detaching, the metric will return 'value' in perpetuity.
  void AutoDetach(FunctionGaugeDetacher* detacher, T value = T()) {
    detacher->OnDestructor(base::Bind(&FunctionGauge<T>::DetachToConstant,
                                this, value));
  }

  // Automatically detach this gauge when the given 'detacher' destructs.
  // After detaching, the metric will return whatever its value was at the
  // time of detaching.
  //
  // Note that, when using this method, you should be sure that the FunctionGaugeDetacher
  // is destructed before any objects which are required by the gauge implementation.
  // In typical usage (see the FunctionGaugeDetacher class documentation) this means you
  // should declare the detacher member after all other class members that might be
  // accessed by the gauge function implementation.
  void AutoDetachToLastValue(FunctionGaugeDetacher* detacher) {
    detacher->OnDestructor(base::Bind(&FunctionGauge<T>::DetachToCurrentValue,
                                this));
  }

 private:
  friend class MetricEntity;

  FunctionGauge(const GaugePrototype<T>* proto, base::Callback<T()> function)
      : Gauge(proto), function_(std::move(function)) {}

  static T Return(T v) {
    return v;
  }

  mutable simple_spinlock lock_;
  base::Callback<T()> function_;
  DISALLOW_COPY_AND_ASSIGN(FunctionGauge);
};

// Prototype for a counter.
class CounterPrototype : public MetricPrototype {
 public:
  explicit CounterPrototype(const MetricPrototype::CtorArgs& args)
    : MetricPrototype(args) {
  }
  scoped_refptr<Counter> Instantiate(const scoped_refptr<MetricEntity>& entity);

  virtual MetricType::Type type() const OVERRIDE { return MetricType::kCounter; }

 private:
  DISALLOW_COPY_AND_ASSIGN(CounterPrototype);
};

// Simple incrementing 64-bit integer.
// Only use Counters in cases that we expect the count to only increase. For example,
// a counter is appropriate for "number of transactions processed by the server",
// but not for "number of transactions currently in flight". Monitoring software
// knows that counters only increase and thus can compute rates over time, rates
// across multiple servers, etc, which aren't appropriate in the case of gauges.
class Counter : public Metric {
 public:
  int64_t value() const;
  void Increment();
  void IncrementBy(int64_t amount);
  virtual Status WriteAsJson(JsonWriter* w,
                             const MetricJsonOptions& opts) const OVERRIDE;

 private:
  FRIEND_TEST(MetricsTest, SimpleCounterTest);
  FRIEND_TEST(MultiThreadedMetricsTest, CounterIncrementTest);
  friend class MetricEntity;

  explicit Counter(const CounterPrototype* proto);

  LongAdder value_;
  DISALLOW_COPY_AND_ASSIGN(Counter);
};

class HistogramPrototype : public MetricPrototype {
 public:
  HistogramPrototype(const MetricPrototype::CtorArgs& args,
                     uint64_t max_trackable_value, int num_sig_digits);
  scoped_refptr<Histogram> Instantiate(const scoped_refptr<MetricEntity>& entity);

  uint64_t max_trackable_value() const { return max_trackable_value_; }
  int num_sig_digits() const { return num_sig_digits_; }
  virtual MetricType::Type type() const OVERRIDE { return MetricType::kHistogram; }

 private:
  const uint64_t max_trackable_value_;
  const int num_sig_digits_;
  DISALLOW_COPY_AND_ASSIGN(HistogramPrototype);
};

class Histogram : public Metric {
 public:
  // Increment the histogram for the given value.
  // 'value' must be non-negative.
  void Increment(int64_t value);

  // Increment the histogram for the given value by the given amount.
  // 'value' and 'amount' must be non-negative.
  void IncrementBy(int64_t value, int64_t amount);

  // Return the total number of values added to the histogram (via Increment()
  // or IncrementBy()).
  uint64_t TotalCount() const;

  virtual Status WriteAsJson(JsonWriter* w,
                             const MetricJsonOptions& opts) const OVERRIDE;

  // Returns a snapshot of this histogram including the bucketed values and counts.
  Status GetHistogramSnapshotPB(HistogramSnapshotPB* snapshot,
                                const MetricJsonOptions& opts) const;

  uint64_t CountInBucketForValueForTests(uint64_t value) const;
  uint64_t MinValueForTests() const;
  uint64_t MaxValueForTests() const;
  double MeanValueForTests() const;

 private:
  FRIEND_TEST(MetricsTest, SimpleHistogramTest);
  friend class MetricEntity;
  explicit Histogram(const HistogramPrototype* proto);

  const gscoped_ptr<HdrHistogram> histogram_;
  DISALLOW_COPY_AND_ASSIGN(Histogram);
};

// Measures a duration while in scope. Adds this duration to specified histogram on destruction.
class ScopedLatencyMetric {
 public:
  // NOTE: the given histogram must live as long as this object.
  // If 'latency_hist' is NULL, this turns into a no-op.
  explicit ScopedLatencyMetric(Histogram* latency_hist);
  ~ScopedLatencyMetric();

 private:
  Histogram* latency_hist_;
  MonoTime time_started_;
};

#define SCOPED_LATENCY_METRIC(_mtx, _h) \
  ScopedLatencyMetric _h##_metric((_mtx) ? (_mtx)->_h.get() : NULL)


////////////////////////////////////////////////////////////
// Inline implementations of template methods
////////////////////////////////////////////////////////////

inline scoped_refptr<Counter> MetricEntity::FindOrCreateCounter(
    const CounterPrototype* proto) {
  CheckInstantiation(proto);
  std::lock_guard<simple_spinlock> l(lock_);
  scoped_refptr<Counter> m = down_cast<Counter*>(FindPtrOrNull(metric_map_, proto).get());
  if (!m) {
    m = new Counter(proto);
    InsertOrDie(&metric_map_, proto, m);
  }
  return m;
}

inline scoped_refptr<Histogram> MetricEntity::FindOrCreateHistogram(
    const HistogramPrototype* proto) {
  CheckInstantiation(proto);
  std::lock_guard<simple_spinlock> l(lock_);
  scoped_refptr<Histogram> m = down_cast<Histogram*>(FindPtrOrNull(metric_map_, proto).get());
  if (!m) {
    m = new Histogram(proto);
    InsertOrDie(&metric_map_, proto, m);
  }
  return m;
}

template<typename T>
inline scoped_refptr<AtomicGauge<T> > MetricEntity::FindOrCreateGauge(
    const GaugePrototype<T>* proto,
    const T& initial_value) {
  CheckInstantiation(proto);
  std::lock_guard<simple_spinlock> l(lock_);
  scoped_refptr<AtomicGauge<T> > m = down_cast<AtomicGauge<T>*>(
      FindPtrOrNull(metric_map_, proto).get());
  if (!m) {
    m = new AtomicGauge<T>(proto, initial_value);
    InsertOrDie(&metric_map_, proto, m);
  }
  return m;
}

template<typename T>
inline scoped_refptr<FunctionGauge<T> > MetricEntity::FindOrCreateFunctionGauge(
    const GaugePrototype<T>* proto,
    const base::Callback<T()>& function) {
  CheckInstantiation(proto);
  std::lock_guard<simple_spinlock> l(lock_);
  scoped_refptr<FunctionGauge<T> > m = down_cast<FunctionGauge<T>*>(
      FindPtrOrNull(metric_map_, proto).get());
  if (!m) {
    m = new FunctionGauge<T>(proto, function);
    InsertOrDie(&metric_map_, proto, m);
  }
  return m;
}

} // namespace mprmpr

#endif // KUDU_UTIL_METRICS_H

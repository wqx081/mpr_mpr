
#include <atomic>
#include <gflags/gflags.h>
#include <glog/logging.h>
#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "mprmpr/rpc/service_queue.h"
#include "mprmpr/util/stopwatch.h"
#include "mprmpr/util/test_util.h"

#include "mprmpr/util/optional.h"

using std::shared_ptr;
using std::string;
using std::unique_ptr;
using std::vector;

DEFINE_int32(num_producers, 4,
             "Number of producer threads");

DEFINE_int32(num_consumers, 20,
             "Number of consumer threads");

DEFINE_int32(max_queue_size, 50,
             "Max queue length");

namespace mprmpr {
namespace rpc {

static std::atomic<uint32_t> inprogress;

static std::atomic<uint32_t> total;

template <typename Queue>
void ProducerThread(Queue* queue) {
  int max_inprogress = FLAGS_max_queue_size - FLAGS_num_producers;
  while (true) {
    while (inprogress > max_inprogress) {
      base::subtle::PauseCPU();
    }
    inprogress++;
    InboundCall* call = new InboundCall(nullptr);
    boost::optional<InboundCall*> evicted;
    auto status = queue->Put(call, &evicted);
    if (status == QUEUE_FULL) {
      LOG(INFO) << "queue full: producer exiting";
      delete call;
      break;
    }

    if (PREDICT_FALSE(evicted != boost::none)) {
      LOG(INFO) << "call evicted: producer exiting";
      delete evicted.get();
      break;
    }

    if (PREDICT_TRUE(status == QUEUE_SHUTDOWN)) {
      delete call;
      break;
    }
  }
}

template <typename Queue>
void ConsumerThread(Queue* queue) {
  unique_ptr<InboundCall> call;
  while (queue->BlockingGet(&call)) {
    inprogress--;
    total++;
    call.reset();
  }
}

TEST(TestServiceQueue, LifoServiceQueuePerf) {
  LifoServiceQueue queue(FLAGS_max_queue_size);
  vector<std::thread> producers;
  vector<std::thread> consumers;

  for (int i = 0; i < FLAGS_num_producers; i++) {
    producers.emplace_back(&ProducerThread<LifoServiceQueue>, &queue);
  }

  for (int i = 0; i < FLAGS_num_consumers; i++) {
    consumers.emplace_back(&ConsumerThread<LifoServiceQueue>, &queue);
  }

  int seconds = AllowSlowTests() ? 10 : 1;
  uint64_t total_sample = 0;
  uint64_t total_queue_len = 0;
  uint64_t total_idle_workers = 0;
  Stopwatch sw(Stopwatch::ALL_THREADS);
  sw.start();
  int32_t before = total;

  for (int i = 0; i < seconds * 50; i++) {
    SleepFor(MonoDelta::FromMilliseconds(20));
    total_sample++;
    total_queue_len += queue.estimated_queue_length();
    total_idle_workers += queue.estimated_idle_worker_count();
  }

  sw.stop();
  int32_t delta = total - before;

  queue.Shutdown();
  for (int i = 0; i < FLAGS_num_producers; i++) {
    producers[i].join();
  }
  for (int i = 0; i < FLAGS_num_consumers; i++) {
    consumers[i].join();
  }

  float reqs_per_second = static_cast<float>(delta / sw.elapsed().wall_seconds());
  float user_cpu_micros_per_req = static_cast<float>(sw.elapsed().user / 1000.0 / delta);
  float sys_cpu_micros_per_req = static_cast<float>(sw.elapsed().system / 1000.0 / delta);

  LOG(INFO) << "Reqs/sec:         " << (int32_t)reqs_per_second;
  LOG(INFO) << "User CPU per req: " << user_cpu_micros_per_req << "us";
  LOG(INFO) << "Sys CPU per req:  " << sys_cpu_micros_per_req << "us";
  LOG(INFO) << "Avg rpc queue length: " << total_queue_len / static_cast<double>(total_sample);
  LOG(INFO) << "Avg idle workers:     " << total_idle_workers / static_cast<double>(total_sample);
}

} // namespace rpc
} // namespace mprmpr

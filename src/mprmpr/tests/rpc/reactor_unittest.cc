#include "mprmpr/rpc/reactor.h"

#include "mprmpr/tests/rpc/rpc-test-base.h"
#include "mprmpr/util/countdown_latch.h"
#include <functional>

namespace mprmpr {
namespace rpc {

class ReactorTest : public RpcTestBase {
 public:
  ReactorTest()
    : messenger_(CreateMessenger("my_messenger", 4)),
      latch_(1) {
  }

  void ScheduledTask(const Status& status, const Status& expected_status) {
    CHECK_EQ(expected_status.CodeAsString(), status.CodeAsString());
    latch_.CountDown();
  }

  void ScheduledTaskCheckThread(const Status& status, const Thread* thread) {
    CHECK_OK(status);
    CHECK_EQ(thread, Thread::current_thread());
    latch_.CountDown();
  }

  void ScheduledTaskScheduleAgain(const Status& status) {
    messenger_->ScheduleOnReactor(
        std::bind(&ReactorTest::ScheduledTaskCheckThread, this, std::placeholders::_1, Thread::current_thread()),
        MonoDelta::FromMilliseconds(0));
    latch_.CountDown();
  }

 protected:
  const std::shared_ptr<Messenger> messenger_;
  CountDownLatch latch_;
};

TEST_F(ReactorTest, TestFunctionIsCalled) {
  messenger_->ScheduleOnReactor(
      std::bind(&ReactorTest::ScheduledTask, this, std::placeholders::_1, Status::OK()),
      MonoDelta::FromSeconds(0));
  latch_.Wait();
}

TEST_F(ReactorTest, TestFunctionIsCalledAtTheRightTime) {
  MonoTime before = MonoTime::Now();
  messenger_->ScheduleOnReactor(
      std::bind(&ReactorTest::ScheduledTask, this, std::placeholders::_1, Status::OK()),
      MonoDelta::FromMilliseconds(100));
  latch_.Wait();
  MonoTime after = MonoTime::Now();
  MonoDelta delta = after - before;
  CHECK_GE(delta.ToMilliseconds(), 100);
}

TEST_F(ReactorTest, TestFunctionIsCalledIfReactorShutdown) {
  messenger_->ScheduleOnReactor(
      std::bind(&ReactorTest::ScheduledTask, this, std::placeholders::_1, Status::Aborted("doesn't matter")),
      MonoDelta::FromSeconds(60));
  messenger_->Shutdown();
  latch_.Wait();
}

TEST_F(ReactorTest, TestReschedulesOnSameReactorThread) {
  latch_.Reset(2);

  messenger_->ScheduleOnReactor(
      std::bind(&ReactorTest::ScheduledTaskScheduleAgain, this, std::placeholders::_1),
      MonoDelta::FromSeconds(0));
  latch_.Wait();
  latch_.Wait();
}

} // namespace rpc
} // namespace mprmpr


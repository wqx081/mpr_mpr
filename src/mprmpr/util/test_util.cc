#include "mprmpr/util/test_util.h"

#include <gflags/gflags.h>
#include <glog/logging.h>
#include <gtest/gtest-spi.h>

#include "mprmpr/base/strings/strcat.h"
#include "mprmpr/base/strings/substitute.h"
#include "mprmpr/base/strings/util.h"
#include "mprmpr/base/walltime.h"
#include "mprmpr/util/env.h"
#include "mprmpr/util/path_util.h"
#include "mprmpr/util/random.h"

DEFINE_string(test_leave_files, "on_failure",
              "Whether to leave test files around after the test run. "
              " Valid values are 'always', 'on_failure', or 'never'");

DEFINE_int32(test_random_seed, 0, "Random seed to use for randomized tests");

using std::string;
using strings::Substitute;

namespace mprmpr {

const char* kInvalidPath = "/dev/invalid-path-for-mprmpr-tests";
static const char* const kSlowTestsEnvVariable = "ANT_ALLOW_SLOW_TESTS";

static const uint64 kTestBeganAtMicros = Env::Default()->NowMicros();

bool g_is_gtest = true;


AntTest::AntTest()
  : env_(Env::Default()),
    test_dir_(GetTestDataDirectory()) {
}   

AntTest::~AntTest() {
  if (FLAGS_test_leave_files == "always") {
    LOG(INFO) << "-----------------------------------------------";
    LOG(INFO) << "--test_leave_files specified, leaving files in " << test_dir_;
  } else if (FLAGS_test_leave_files == "on_failure" && HasFatalFailure()) {
    LOG(INFO) << "-----------------------------------------------";
    LOG(INFO) << "Had fatal failures, leaving test files at " << test_dir_;
  } else {
    VLOG(1) << "Cleaning up temporary test files...";
    WARN_NOT_OK(env_->DeleteRecursively(test_dir_),
                                "Couldn't remove test files");
  }             
} 

void AntTest::SetUp() {
////TODO(wqx):
////  InitSpinLockContentionProfiling();
  OverrideKrb5Environment();
} 

string AntTest::GetTestPath(const string& relative_path) {
  return JoinPathSegments(test_dir_, relative_path);
} 

void AntTest::OverrideKrb5Environment() {
  setenv("KRB5_CONFIG", kInvalidPath, 1);
  setenv("KRB5_KTNAME", kInvalidPath, 1);
  setenv("KRB5CCNAME", kInvalidPath, 1);
} 

bool AllowSlowTests() {
  char *e = getenv(kSlowTestsEnvVariable);
  if ((e == nullptr) ||
            (strlen(e) == 0) ||
            (strcasecmp(e, "false") == 0) ||
            (strcasecmp(e, "0") == 0) ||
            (strcasecmp(e, "no") == 0)) {
    return false;
  } 
  if ((strcasecmp(e, "true") == 0) ||
            (strcasecmp(e, "1") == 0) || 
            (strcasecmp(e, "yes") == 0)) {
    return true;
  } 
  LOG(FATAL) << "Unrecognized value for " << kSlowTestsEnvVariable << ": " << e;
  return false; 
} 

void OverrideFlagForSlowTests(const std::string& flag_name,
                                                            const std::string& new_value) {
  google::GetCommandLineFlagInfoOrDie(flag_name.c_str());
  
  if (!AllowSlowTests()) {
    return;
  }
  google::SetCommandLineOptionWithMode(flag_name.c_str(), new_value.c_str(),
                                                                              google::SET_FLAG_IF_DEFAULT);
}

int SeedRandom() {
  int seed;
  if (FLAGS_test_random_seed == 0) {
    seed = static_cast<int>(GetCurrentTimeMicros());
  } else { 
              seed = FLAGS_test_random_seed;
            } 
  LOG(INFO) << "Using random seed: " << seed;
  srand(seed); 
  return seed;
} 

string GetTestDataDirectory() {
  const ::testing::TestInfo* const test_info =
    ::testing::UnitTest::GetInstance()->current_test_info();
  CHECK(test_info) << "Must be running in a gtest unit test to call this function";
  string dir;
  CHECK_OK(Env::Default()->GetTestDirectory(&dir));
  
  dir += Substitute("/$0.$1.$2.$3-$4",
                        StringReplace(google::ProgramInvocationShortName(), "/", "_", true),
                        StringReplace(test_info->test_case_name(), "/", "_", true),
                        StringReplace(test_info->name(), "/", "_", true),
                        kTestBeganAtMicros,
                        getpid());
  Status s = Env::Default()->CreateDir(dir);
  CHECK(s.IsAlreadyPresent() || s.ok())
    << "Could not create directory " << dir << ": " << s.ToString();
  if (s.ok()) {
    string metadata;
    
    StrAppend(&metadata, Substitute("PID=$0\n", getpid()));
    
    StrAppend(&metadata, Substitute("PPID=$0\n", getppid()));
    
    char* jenkins_build_id = getenv("BUILD_ID");
    if (jenkins_build_id) {
      StrAppend(&metadata, Substitute("BUILD_ID=$0\n", jenkins_build_id));
    } 
    
    CHECK_OK(WriteStringToFile(Env::Default(), metadata,
                                                              Substitute("$0/test_metadata", dir)));
  }                            
  return dir;                  
} 

void AssertEventually(const std::function<void(void)>& f,
                                            const MonoDelta& timeout) {
  const MonoTime deadline = MonoTime::Now() + timeout;
  
  for (int attempts = 0; MonoTime::Now() < deadline; attempts++) {
    testing::TestPartResultArray results;
    testing::ScopedFakeTestPartResultReporter reporter(
        testing::ScopedFakeTestPartResultReporter::INTERCEPT_ONLY_CURRENT_THREAD,
        &results);
    f();

    bool has_failures = false;
    for (int i = 0; i < results.size(); i++) {
      has_failures |= results.GetTestPartResult(i).failed();
    }
    if (!has_failures) {
      return;
    }

    int sleep_ms = (attempts < 10) ? (1 << attempts) : 1000;
    SleepFor(MonoDelta::FromMilliseconds(sleep_ms));
  }

  f();
  if (testing::Test::HasFatalFailure()) {
    ADD_FAILURE() << "Timed out waiting for assertion to pass.";
  }
}

} // namespace mprmpr

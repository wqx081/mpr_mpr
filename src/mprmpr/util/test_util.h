#ifndef ANT_UTIL_TEST_UTIL_H_
#define ANT_UTIL_TEST_UTIL_H_

#include <functional>
#include <gtest/gtest.h>
#include <string>

#include "mprmpr/base/gscoped_ptr.h"
#include "mprmpr/util/env.h"
#include "mprmpr/util/monotime.h"
#include "mprmpr/util/test_macros.h"

namespace mprmpr {

extern const char* kInvalidPath;

class AntTest : public ::testing::Test {
 public:
  AntTest();

  virtual ~AntTest();

  virtual void SetUp() override;

  static void OverrideKrb5Environment();

 protected:
  std::string GetTestPath(const std::string& relative_path);
  
  Env* env_;
  google::FlagSaver flag_saver_;  // Reset flags on every test.
  std::string test_dir_;
};

bool AllowSlowTests();
void OverrideFlagForSlowTests(const std::string& flag_name,
                              const std::string& new_value);
int SeedRandom();
std::string GetTestDataDirectory();
void AssertEventually(const std::function<void(void)>& f,
                      const MonoDelta& timeout = MonoDelta::FromSeconds(30));


} // namespace mprmpr
#endif // ANT_UTIL_TEST_UTIL_H_

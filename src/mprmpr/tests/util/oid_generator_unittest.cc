#include "mprmpr/util/oid_generator.h"

#include <gtest/gtest.h>
#include <string>

#include "mprmpr/util/test_util.h"

using std::string;

namespace mprmpr {

TEST(ObjectIdGeneratorTest, TestCanoicalizeUuid) {
  ObjectIdGenerator gen;
  const string kExpectedCanonicalized = "0123456789abcdef0123456789abcdef";
  string canonicalized;
  Status s = gen.Canonicalize("not_a_uuid", &canonicalized);
  {
    SCOPED_TRACE(s.ToString());
    ASSERT_TRUE(s.IsInvalidArgument());
    ASSERT_STR_CONTAINS(s.ToString(), "invalid uuid");
  }
  ASSERT_OK(gen.Canonicalize(
      "01234567-89ab-cdef-0123-456789abcdef", &canonicalized));
  ASSERT_EQ(kExpectedCanonicalized, canonicalized);
  ASSERT_OK(gen.Canonicalize(
      "0123456789abcdef0123456789abcdef", &canonicalized));
  ASSERT_EQ(kExpectedCanonicalized, canonicalized);
  ASSERT_OK(gen.Canonicalize(
      "0123456789AbCdEf0123456789aBcDeF", &canonicalized));
  ASSERT_EQ(kExpectedCanonicalized, canonicalized);
}

} // namespace mprmpr

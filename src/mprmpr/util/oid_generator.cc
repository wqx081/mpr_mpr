#include "mprmpr/util/oid_generator.h"

#include <boost/uuid/uuid_generators.hpp>
#include <exception>
#include <mutex>
#include <string>

#include "mprmpr/base/stringprintf.h"
#include "mprmpr/base/strings/substitute.h"
#include "mprmpr/util/status.h"

using strings::Substitute;

namespace mprmpr {

namespace {

string ConvertUuidToString(const boost::uuids::uuid& to_convert) {
  const uint8_t* uuid = to_convert.data;
  return StringPrintf("%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
               uuid[0], uuid[1], uuid[2], uuid[3], uuid[4], uuid[5], uuid[6], uuid[7],
               uuid[8], uuid[9], uuid[10], uuid[11], uuid[12], uuid[13], uuid[14], uuid[15]);
}

} // anonymous namespace

string ObjectIdGenerator::Next() {
  std::lock_guard<LockType> l(oid_lock_);
  boost::uuids::uuid uuid = oid_generator_();
  return ConvertUuidToString(uuid);
}

Status ObjectIdGenerator::Canonicalize(const string& input,
                                       string* output) const {
  try {
    boost::uuids::uuid uuid = oid_validator_(input);
    *output = ConvertUuidToString(uuid);
    return Status::OK();
  } catch (std::exception& e) {
    return Status::InvalidArgument(Substitute("invalid uuid $0: $1",
                                              input, e.what()));
  }
}

} // namespace mprmpr

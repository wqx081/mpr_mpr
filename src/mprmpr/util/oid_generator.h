#ifndef KUDU_UTIL_OID_GENERATOR_H
#define KUDU_UTIL_OID_GENERATOR_H

#include <boost/uuid/uuid_generators.hpp>
#include <string>

#include "mprmpr/base/macros.h"
#include "mprmpr/util/locks.h"
#include "mprmpr/util/status.h"

namespace mprmpr {

// Generates a unique 32byte id, based on uuid v4.
// This class is thread safe
class ObjectIdGenerator {
 public:
  ObjectIdGenerator() {}
  ~ObjectIdGenerator() {}

  // Generates and returns a new UUID.
  std::string Next();

  // Validates an existing UUID and converts it into the format used by Kudu
  // (that is, 16 hexadecimal bytes without any dashes).
  Status Canonicalize(const std::string& input, std::string* output) const;

 private:
  DISALLOW_COPY_AND_ASSIGN(ObjectIdGenerator);

  typedef simple_spinlock LockType;

  // Protects 'oid_generator_'.
  LockType oid_lock_;

  // Generates new UUIDs.
  boost::uuids::random_generator oid_generator_;

  // Validates provided UUIDs.
  boost::uuids::string_generator oid_validator_;
};

} // namespace mprmpr

#endif

#ifndef MPRMPR_DB_RESULT_FIELD_IMPL_H_
#define MPRMPR_DB_RESULT_FIELD_IMPL_H_

#include <stdint.h>
#include <ctime>
#include <string>

#include "mprmpr/base/int128.h"

namespace mprmpr {
namespace db {

class ResultFieldImpl {
 public:
  virtual ~ResultFieldImpl() {}

  virtual bool IsNULL() const = 0;

  virtual operator int8_t() const = 0;
  virtual operator int16_t() const = 0;
  virtual operator uint16_t() const = 0;
  virtual operator int32_t() const = 0;
  virtual operator uint32_t() const = 0;
  virtual operator int64_t() const = 0;
  virtual operator uint64_t() const = 0;
  virtual operator float() const = 0;
  virtual operator double() const = 0;
  virtual operator uint128() const = 0;

  virtual operator std::string() const = 0;
  virtual operator std::tm() const = 0;
};

} // namespace db
} // namespace mprmpr
#endif // MPRMPR_DB_RESULT_FIELD_IMPL_H_

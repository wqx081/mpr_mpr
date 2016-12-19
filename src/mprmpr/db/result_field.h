#ifndef MPRMPR_DB_RESULT_FIELD_H_
#define MPRMPR_DB_RESULT_FIELD_H_

#include <memory>
#include <utility>
#include <ctime>
#include <iostream>

#include "mprmpr/base/int128.h"

namespace mprmpr {
namespace db {

class ResultImpl;
class ResultFieldImpl;

class ResultField {
 public:
  explicit ResultField(std::shared_ptr<ResultImpl> result, ResultFieldImpl* field);
  bool IsNull() const;
  
  operator int8_t() const;
  operator uint16_t() const;
  operator int16_t() const;
  operator uint32_t() const;
  operator int32_t() const;
  operator int64_t() const;
  operator uint64_t() const;
  operator float() const;
  operator double() const;

  operator uint128() const;

  operator std::string() const;
  operator std::tm() const;
  
 private:
  std::shared_ptr<ResultImpl> result_;
  ResultFieldImpl* field_;
};

std::ostream& operator<<(std::ostream& stream, const ResultField& field);

} // namespace db
} // namespace mprmpr
#endif // MPRMPR_DB_RESULT_FIELD_H_

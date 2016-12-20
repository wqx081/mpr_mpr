#include "mprmpr/db/result_field.h"
#include "mprmpr/db/result_field_impl.h"
#include "mprmpr/db/result_impl.h"

namespace mprmpr {
namespace db {

ResultField::ResultField(std::shared_ptr<ResultImpl> result, ResultFieldImpl* field)
    : result_(result),
      field_(field) {}

bool ResultField::IsNULL() const {
  return field_ == nullptr || field_->IsNULL();
}

ResultField::operator int8_t()   const { return *field_; }
ResultField::operator uint16_t() const { return *field_; }
ResultField::operator int16_t()  const { return *field_; }
ResultField::operator uint32_t() const { return *field_; }
ResultField::operator int32_t()  const { return *field_; }
ResultField::operator uint64_t() const { return *field_; }
ResultField::operator int64_t()  const { return *field_; }
ResultField::operator float()    const { return *field_; }
ResultField::operator double()   const { return *field_; }

ResultField::operator uint128() const { return *field_; }

ResultField::operator std::string() const { return *field_; }
ResultField::operator std::tm() const { return *field_; }

std::ostream& operator<<(std::ostream& stream, const ResultField& field) {
  if (field.IsNULL()) {
    return stream << "(NULL)";
  } else {
    return stream << (std::string) field;
  }
}

} // namespace db
} // namespace mprmpr

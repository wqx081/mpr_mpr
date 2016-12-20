#ifndef MPRMPR_DB_STATEMENT_INTEGRAL_RESULT_FIELD_H_
#define MPRMPR_DB_STATEMENT_INTEGRAL_RESULT_FIELD_H_

#include "mprmpr/db/statement_result_field.h"

namespace mprmpr {
namespace db {

template<typename T>
class StatementIntegralResultField : public StatementResultField {
 public:
  StatementIntegralResultField() : StatementResultField() {}

  virtual operator int8_t()    const override { return IsNULL() ? 0 : value_; }
  virtual operator uint16_t()  const override { return IsNULL() ? 0 : value_; }
  virtual operator int16_t()   const override { return IsNULL() ? 0 : value_; }
  virtual operator uint32_t()  const override { return IsNULL() ? 0 : value_; }
  virtual operator int32_t()   const override { return IsNULL() ? 0 : value_; }
  virtual operator uint64_t()  const override { return IsNULL() ? 0 : value_; }
  virtual operator int64_t()   const override { return IsNULL() ? 0 : value_; }
  virtual operator float()     const override { return IsNULL() ? 0 : value_; }
  virtual operator double()    const override { return IsNULL() ? 0 : value_; }
  virtual operator uint128()   const override { return IsNULL() ? 0 : (uint64_t) value_; }

  virtual operator std::string() const override { return std::to_string(value_); }
  virtual operator std::tm() const override { return std::tm{}; }

 private:
  T value_;
  virtual void* GetValue() override {
    return static_cast<void*>(&value_);
  }
};

using StatementSignedCharResultField        =   StatementIntegralResultField<int8_t>;
using StatementUnsignedShortResultField     =   StatementIntegralResultField<uint16_t>;
using StatementSignedShortResultField       =   StatementIntegralResultField<int16_t>;
using StatementUnsignedLongResultField      =   StatementIntegralResultField<uint32_t>;
using StatementSignedLongResultField        =   StatementIntegralResultField<int32_t>;
using StatementUnsignedLongLongResultField  =   StatementIntegralResultField<uint64_t>;
using StatementSignedLongLongResultField    =   StatementIntegralResultField<int64_t>;
using StatementFloatResultField             =   StatementIntegralResultField<float>;
using StatementDoubleResultField            =   StatementIntegralResultField<double>;

} // namespace db
} // namespace mprmpr
#endif // MPRMPR_DB_STATEMENT_INTEGRAL_RESULT_FIELD_H_

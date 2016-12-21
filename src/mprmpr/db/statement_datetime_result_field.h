#ifndef MPRMPR_DB_STATEMENT_DATETIME_RESULT_FIELD_H_
#define MPRMPR_DB_STATEMENT_DATETIME_RESULT_FIELD_H_

#include <glog/logging.h>

#include "mprmpr/db/statement_result_field.h"

namespace mprmpr {
namespace db {

class StatementDatetimeResultField : public StatementResultField {
 public:
  StatementDatetimeResultField() : StatementResultField() {}

  virtual operator int8_t()    const override { return 0; }
  virtual operator uint16_t()  const override { return 0; }
  virtual operator int16_t()   const override { return 0; }
  virtual operator uint32_t()  const override { return 0; }
  virtual operator int32_t()   const override { return 0; }
  virtual operator uint64_t()  const override { return 0; }
  virtual operator int64_t()   const override { return 0; }
  virtual operator float()     const override { return 0; }
  virtual operator double()    const override { return 0; }
  virtual operator uint128()   const override { return 0; }

  virtual operator std::string() const override { 
    std::tm datetime = *this;
    char buf[64] = {0};
    std::strftime(buf, 64, "%Y-%m-%d %H:%M:%S", &datetime);
    return std::string(buf);
  }
  
  virtual operator std::tm() const override {
    return std::tm { (int)value_.second, 
                     (int)value_.minute, 
                     (int)value_.hour, 
                     (int)value_.day, 
                     (int)value_.month - 1, 
                     (int)value_.year - 1900, 0, 0, -1 };
  }

 private:
  MYSQL_TIME value_;

  virtual void* GetValue() override {
    return static_cast<void*>(&value_);
  }
};

} // namespace db
} // namespace mprmpr
#endif // MPRMPR_DB_STATEMENT_DATETIME_RESULT_FIELD_H_

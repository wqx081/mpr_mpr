#ifndef MPRMPR_DB_STATEMENT_RESULT_FIELD_H_
#define MPRMPR_DB_STATEMENT_RESULT_FIELD_H_

#include <mysql/mysql.h>

#include "mprmpr/db/result_field_impl.h"

namespace mprmpr {
namespace db {

class StatementResultField : public ResultFieldImpl {

  friend class StatementResultInfo;

 public:
  StatementResultField() : null_(false) {}
  virtual ~StatementResultField() {}

  bool IsNULL() const { return null_; }

  virtual operator int8_t()    const override = 0;
  virtual operator uint16_t()  const override = 0;
  virtual operator int16_t()   const override = 0;
  virtual operator uint32_t()  const override = 0;
  virtual operator int32_t()   const override = 0;
  virtual operator uint64_t()  const override = 0;
  virtual operator int64_t()   const override = 0;
  virtual operator float()     const override = 0;
  virtual operator double()    const override = 0;
  virtual operator uint128() const override = 0;
  
  virtual operator std::string() const override = 0;
  virtual operator std::tm() const override = 0;
      
 protected:

  virtual bool IsDynamic() {
    return false;
  }

 private:
  my_bool null_;
  virtual void* GetValue() = 0;
  my_bool* GetNull() { return &null_; }
};

} // namespace db
} // namespace mprmpr
#endif // MPRMPR_DB_STATEMENT_RESULT_FIELD_H_

#ifndef MPRMPR_DB_STATEMENT_DYNAMIC_RESULT_FIELD_H_
#define MPRMPR_DB_STATEMENT_DYNAMIC_RESULT_FIELD_H_

#include <glog/logging.h>
#include "mprmpr/db/statement_result_field.h"

namespace mprmpr {
namespace db {

class StatementDynamicResultField : public StatementResultField {
  friend class StatementResultInfo;
 public:
  StatementDynamicResultField() : StatementResultField(), value_(nullptr), size_(0) {}

  virtual ~StatementDynamicResultField() {
    std::free(value_);
  }

  virtual operator int8_t()   const override { return IsNULL() ? 0 : std::stoi(value_);  }
  virtual operator uint16_t() const override { return IsNULL() ? 0 : std::stoi(value_);  }
  virtual operator int16_t()  const override { return IsNULL() ? 0 : std::stoi(value_);  }
  virtual operator uint32_t() const override { return IsNULL() ? 0 : std::stoul(value_);  }
  virtual operator int32_t()  const override { return IsNULL() ? 0 : std::stoi(value_);  }
  virtual operator uint64_t() const override { return IsNULL() ? 0 : std::stoull(value_); }
  virtual operator int64_t()  const override { return IsNULL() ? 0 : std::stoll(value_);  }
  virtual operator float()    const override { return IsNULL() ? 0 : std::stof(value_);  }
  virtual operator double()   const override { return IsNULL() ? 0 : std::stod(value_);  }

  virtual operator uint128() const override {
#if 0
    DCHECK(size_ == sizeof(uint128));
    uint128 output;
#endif
    DCHECK(false) << "TODO(wqx)";
  }

  virtual operator std::string() const override {
    return std::string(value_, size_);
  }

  virtual operator std::tm() const override {
    return std::tm{};
  }

 protected:

  virtual bool IsDynamic() override {
    return true;
  }

 private:
  char* value_;
  unsigned long size_;

  virtual void* GetValue() override {
    return static_cast<void*>(value_);
  }
};

} // namespace db
} // namespace mprmpr
#endif // MPRMPR_DB_STATEMENT_DYNAMIC_RESULT_FIELD_H_

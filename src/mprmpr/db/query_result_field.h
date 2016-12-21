#ifndef MPRMPR_DB_QUERY_RESULT_FIELD_H_
#define MPRMPR_DB_QUERY_RESULT_FIELD_H_

#include <glog/logging.h>
#include "mprmpr/db/result_field_impl.h"

namespace mprmpr {
namespace db {

class QueryResultField : public ResultFieldImpl {

 public:

  QueryResultField(const char* data, size_t length) : data_(data), length_(length) {}

  virtual bool IsNULL() const override {
    return data_ == nullptr;
  }

  virtual operator int8_t()    const override { return IsNULL() ? 0 : std::stoi(data_);  }
  virtual operator uint16_t()  const override { return IsNULL() ? 0 : std::stoi(data_);  }
  virtual operator int16_t()   const override { return IsNULL() ? 0 : std::stoi(data_);  }
  virtual operator uint32_t()  const override { return IsNULL() ? 0 : std::stoul(data_); }
  virtual operator int32_t()   const override { return IsNULL() ? 0 : std::stoi(data_);  }
  virtual operator uint64_t()  const override { return IsNULL() ? 0 : std::stoull(data_);}
  virtual operator int64_t()   const override { return IsNULL() ? 0 : std::stoll(data_); }
  virtual operator float()     const override { return IsNULL() ? 0 : std::stof(data_);  }
  virtual operator double()    const override { return IsNULL() ? 0 : std::stod(data_);  }

  virtual operator uint128() const override {
    DCHECK(false) << "TODO(wqx)";
  }

  virtual operator std::string() const override {
    return IsNULL() ? "" : std::string(data_, length_);
  }
  
  virtual operator std::tm() const override {
    return std::tm{};
  }   

 private:
  const char* data_;
  size_t length_;
};

} // namespace db
} // namespace mprmpr
#endif // MPRMPR_DB_QUERY_RESULT_FIELD_H_

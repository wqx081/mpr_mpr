#include "mprmpr/db/local_parameter.h"

#include <cstdlib>

namespace mprmpr {
namespace db {

LocalParameter::LocalParameter(uint8_t v)   : value_(std::to_string(v)), integral_(true), buffer_(nullptr) {}
LocalParameter::LocalParameter(int8_t v)    : value_(std::to_string(v)), integral_(true), buffer_(nullptr) {}
LocalParameter::LocalParameter(uint16_t v)  : value_(std::to_string(v)), integral_(true), buffer_(nullptr) {}
LocalParameter::LocalParameter(int16_t v)   : value_(std::to_string(v)), integral_(true), buffer_(nullptr) {}
LocalParameter::LocalParameter(uint32_t v)  : value_(std::to_string(v)), integral_(true), buffer_(nullptr) {}
LocalParameter::LocalParameter(int32_t v)   : value_(std::to_string(v)), integral_(true), buffer_(nullptr) {}
LocalParameter::LocalParameter(uint64_t v)  : value_(std::to_string(v)), integral_(true), buffer_(nullptr) {}
LocalParameter::LocalParameter(int64_t v)   : value_(std::to_string(v)), integral_(true), buffer_(nullptr) {}
LocalParameter::LocalParameter(float v)     : value_(std::to_string(v)), integral_(true), buffer_(nullptr) {}
LocalParameter::LocalParameter(double v)    : value_(std::to_string(v)), integral_(true), buffer_(nullptr) {}

LocalParameter::LocalParameter(const std::string& v)
    : value_(v),
      integral_(false),
      buffer_(static_cast<char*>(std::malloc(size()))) {}

LocalParameter::LocalParameter(const char* v)
    : value_(v),
      integral_(false),
      buffer_(static_cast<char*>(std::malloc(size()))) {}

LocalParameter::LocalParameter(std::nullptr_t v)
    : value_("NULL"),
      integral_(true),
      buffer_(nullptr) {}

LocalParameter::~LocalParameter() {
  if (buffer_) {
    std::free(buffer_);
  }
}

size_t LocalParameter::size() const {
  if (integral_) 
    return value_.size();
  else
    return value_.size() * 2 + 2;
}

const std::string LocalParameter::Escape(MYSQL* conn) {
  if (integral_) return value_;
  auto len = ::mysql_real_escape_string(conn, buffer_, value_.c_str(), value_.size());
  return std::string(buffer_, len);
}

const std::string LocalParameter::Quote(MYSQL* conn) {
  if (integral_) return value_;
  buffer_[0] = '\'';
  auto len = ::mysql_real_escape_string(conn, buffer_ + 1, value_.c_str(), value_.size());
  buffer_[len + 1] = '\'';
  buffer_[len + 2] = '\0';
  return std::string(buffer_, len + 2);
}

} // namespace db
} // namespace mprmpr

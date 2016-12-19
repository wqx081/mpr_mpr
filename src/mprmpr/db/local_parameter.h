#ifndef MPRMPR_DB_LOCAL_PARAMETER_H_
#define MPRMPR_DB_LOCAL_PARAMETER_H_

#include <string>
#include <mysql/mysql.h>

namespace mprmpr {
namespace db {

class LocalParameter {
 public:
  explicit LocalParameter(uint8_t v);
  explicit LocalParameter(int8_t v);
  explicit LocalParameter(uint16_t v);
  explicit LocalParameter(int16_t v);
  explicit LocalParameter(uint32_t v);
  explicit LocalParameter(int32_t v);
  explicit LocalParameter(uint64_t v);
  explicit LocalParameter(int64_t v);
  explicit LocalParameter(float v);
  explicit LocalParameter(double v);
 
  explicit LocalParameter(const std::string& v);
  explicit LocalParameter(const char* v);
  explicit LocalParameter(std::nullptr_t v);

  virtual ~LocalParameter();

  size_t size() const;
  const std::string Escape(MYSQL* conn);
  const std::string Quote(MYSQL* conn);
  
 private:
  std::string value_;
  bool integral_;
  char* buffer_;
};

} // namespace db
} // namespace mprmpr
#endif // MPRMPR_DB_LOCAL_PARAMETER_H_

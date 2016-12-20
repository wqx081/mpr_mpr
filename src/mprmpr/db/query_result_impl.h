#ifndef MPRMPR_DB_QUERY_RESULT_IMPL_H_
#define MPRMPR_DB_QUERY_RESULT_IMPL_H_

#include <mysql/mysql.h>

#include "mprmpr/db/result_impl.h"

namespace mprmpr {
namespace db {

class QueryResultImpl : public ResultImpl {
 public:

  QueryResultImpl(MYSQL_RES* result) 
    : ResultImpl(),
      result_(result),
      rows_(::mysql_num_rows(result_)),
      position_(0) {
    auto size = ::mysql_num_fields(result_);
    for (size_t i=0; i < size; ++i) {
      auto field = ::mysql_fetch_field_direct(result_, i);
      fields_[std::string(field->name, field->name_length)] = i;
    }
  }

  virtual ~QueryResultImpl() {
    ::mysql_free_result(result_);
  }

  const std::map<std::string, size_t>& Fields() const override;
  size_t size() const override;
  const std::vector<std::unique_ptr<ResultFieldImpl>>& Fetch(size_t index) override;

 private:
  MYSQL_RES* result_;
  std::vector<std::vector<std::unique_ptr<ResultFieldImpl>>> rows_;
  std::map<std::string, size_t> fields_;
  size_t position_;
};

} // namespace db
} // namespace mprmpr
#endif // MPRMPR_DB_QUERY_RESULT_IMPL_H_

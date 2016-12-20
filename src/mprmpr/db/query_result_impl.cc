#include <glog/logging.h>

#include "mprmpr/db/query_result_impl.h"
#include "mprmpr/db/result_field_impl.h"
#include "mprmpr/db/query_result_field.h"

namespace mprmpr {
namespace db {

const std::map<std::string, size_t>& QueryResultImpl::Fields() const {
  return fields_;
}

size_t QueryResultImpl::size() const {
  return rows_.size();
}

const std::vector<std::unique_ptr<ResultFieldImpl>>& QueryResultImpl::Fetch(size_t index) {
  DCHECK(index <= size());
  if (rows_[index].size() == 0) {
    if (position_ != index) {
      ::mysql_data_seek(result_, index);
    }
    position_ = index + 1;
    auto row = ::mysql_fetch_row(result_);
    auto lens = ::mysql_fetch_lengths(result_);

    rows_[index].reserve(fields_.size());
    for (size_t i=0; i < fields_.size(); ++i) {
      rows_[index].emplace_back(new QueryResultField(row[i], lens[i]));
    }
  }
  return rows_[index];
}

} // namespace db
} // namespace mprmpr

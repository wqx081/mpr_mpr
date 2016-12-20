#include "mprmpr/db/result.h"
#include "mprmpr/db/result_impl.h"
#include "mprmpr/db/query_result_impl.h"

namespace mprmpr {
namespace db {

Result::Result(MYSQL_RES* result)
    : result_(std::make_shared<QueryResultImpl>(result)) {}

Result::Result(std::shared_ptr<ResultImpl>&& impl)
    : result_(std::move(impl)) {}

Result::Result(size_t affected_rows, uint64_t insert_id)
    : affected_rows_(affected_rows),
      insert_id_(insert_id) {}

Result::Result(std::nullptr_t result) {}

Result::Result(Result&& other)
    : result_(std::move(other.result_)),
      affected_rows_(other.affected_rows_) {}

Result::~Result() {}

bool Result::Valid() const {
  return affected_rows_ || result_;
}

size_t Result::affected_rows() const {
  return affected_rows_;
}

// Iterator
Result::Iterator::Iterator(std::shared_ptr<ResultImpl> result, size_t index)
    : result_(result),
      index_(index) {}

Result::Iterator::Iterator(const Result::Iterator& other)
    : result_(other.result_),
      index_(other.index_) {}

bool Result::Iterator::Valid() const {
  return result_ && index_ < result_->size();
}

ResultRow Result::Iterator::operator*() {
  return ResultRow(result_, result_->Fetch(index_));
}

std::unique_ptr<ResultRow> Result::Iterator::operator->() {
  return std::unique_ptr<ResultRow>(new ResultRow(result_, result_->Fetch(index_)));
}

} // namespace db
} // namespace mprmpr

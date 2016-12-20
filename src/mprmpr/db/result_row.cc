#include <glog/logging.h>

#include "mprmpr/db/result_row.h"
#include "mprmpr/db/result_impl.h"
#include "mprmpr/db/result_field_impl.h"

namespace mprmpr {
namespace db {

ResultRow::ResultRow(std::shared_ptr<ResultImpl> result,
                     const std::vector<std::unique_ptr<ResultFieldImpl>>& fields)
    : result_(result),
      fields_(fields) {}

ResultRow::~ResultRow() {
}

size_t ResultRow::size() const {
  return result_->Fields().size();     
}

const ResultField ResultRow::operator[](size_t index) const {
  DCHECK(index <= size());
  return ResultField(result_, fields_[index].get());
}

const ResultField ResultRow::operator[](const std::string& key) const {
  auto it = result_->Fields().find(key);
  DCHECK(it != result_->Fields().end());
  return ResultField(result_, fields_[it->second].get());
}

ResultRow::Iterator ResultRow::begin() const {
  return Iterator(result_->Fields().cbegin(), this);
}

ResultRow::Iterator ResultRow::find(const std::string& key) const {
  return Iterator(result_->Fields().find(key), this);
}

ResultRow::Iterator ResultRow::end() const {
  return Iterator(result_->Fields().cend(), this);
}

/////// iterator

ResultRow::Iterator::Iterator()
    : iterator_(std::map<std::string, size_t>::const_iterator()),
      row_(nullptr) {}

ResultRow::Iterator::Iterator(std::map<std::string, size_t>::const_iterator&& iterator,
                              const ResultRow* row)
    : iterator_(std::move(iterator)),
      row_(row) {}

ResultRow::Iterator::Iterator(const Iterator& other)
    : iterator_(other.iterator_),
      row_(other.row_) {}

ResultRow::Iterator::~Iterator() {}


std::pair<std::string, ResultField> ResultRow::Iterator::operator*() const {
  return std::make_pair<>(iterator_->first, row_->operator[](iterator_->first));
}

std::unique_ptr<std::pair<std::string, ResultField>> 
ResultRow::Iterator::operator->() const {
  return std::unique_ptr<std::pair<std::string, ResultField>>
      (new std::pair<std::string, ResultField>(iterator_->first, row_->operator[](iterator_->first)));  
}

} // namespace db
} // namespace mprmpr

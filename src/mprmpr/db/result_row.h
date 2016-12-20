#ifndef MPRMPR_DB_RESULT_ROW_H_
#define MPRMPR_DB_RESULT_ROW_H_

#include <utility>
#include <memory>
#include <vector>
#include <map>
#include <string>

#include <stdint.h>

#include "mprmpr/db/result_field.h"

namespace mprmpr {
namespace db {

class ResultImpl;
class ResultFieldImpl;

class ResultRow {
 private:
  class Iterator;

 public:

  ResultRow(std::shared_ptr<ResultImpl> result,
            const std::vector<std::unique_ptr<ResultFieldImpl>>& fields);
  virtual ~ResultRow();
  size_t size() const;
  const ResultField operator[](size_t index) const;
  const ResultField operator[](const std::string& key) const;
  Iterator begin() const;
  Iterator end() const;
  Iterator find(const std::string& key) const;

 private:
  class Iterator {
   public:
    Iterator();
    Iterator(std::map<std::string, size_t>::const_iterator&& iterator,
             const ResultRow*);
    Iterator(const Iterator& other);
    virtual ~Iterator();

    Iterator& operator=(const Iterator& other) {
      if (this != &other) {
        iterator_ = other.iterator_;
        row_ = other.row_;
      }
      return *this;
    }

    Iterator& operator++() {
      iterator_++;
      return *this;
    }

    Iterator operator++(int) {
      Iterator it(*this);
      iterator_++;
      return it;
    }

    Iterator& operator--() {
      iterator_--;
      return *this;
    }

    Iterator operator--(int) {
      Iterator it(*this);
      iterator_--;
      return it;
    }

    bool operator==(const Iterator& other) const { return iterator_ == other.iterator_; }
    bool operator!=(const Iterator& other) const { return !(*this == other); }

    std::pair<std::string, ResultField> operator*() const;
    std::unique_ptr<std::pair<std::string, ResultField>> operator->() const;

   private:
    std::map<std::string, size_t>::const_iterator iterator_;
    const ResultRow* row_;
  };

 private:
  std::shared_ptr<ResultImpl> result_;
  const std::vector<std::unique_ptr<ResultFieldImpl>>& fields_;
};

} // namespace db
} // namespace mprmpr

#endif // MPRMPR_DB_RESULT_ROW_H_

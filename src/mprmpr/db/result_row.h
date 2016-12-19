#ifndef MPRMPR_DB_RESULT_ROW_H_
#define MPRMPR_DB_RESULT_ROW_H_

#include <utility>
#include <memory>
#include <vector>
#include <map>
#include <string>

#include <stdint.h>

// TODO(wqx):
// #include "mprmpr/db/result_field.h"
//

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
  // const ResultField operator[](size_t index) const;
  // const ResultField operator[](const std::string& key) const;
  Iterator begin() const;
  Iterator end() const;
  Iterator find(const std::string& key) const;

 private:
  class Iterator {
   public:
    Iterator();
    Iterator(std::map<std::string, size_t>::const_pointer&& iterator,
             const ResultRow);
    Iterator(const Iterator& other);
    virtual ~Iterator();
    Iterator& operator=(const Iterator& other);
    Iterator& operator++();
    Iterator operator++(int);
    Iterator& operator--();
    Iterator operator--(int);
    bool operator==(const Iterator& other) const;
    bool operator!=(const Iterator& other) const;
    //TODO(wqx):
    // std::pair<std::string, ResultField> operator*() const;
    // std::unique_ptr<std::pair<std::string, ResultField>> operator->() const
    //
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

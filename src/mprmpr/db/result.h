#ifndef MPRMPR_DB_RESULT_H_
#define MPRMPR_DB_RESULT_H_

#include <stdint.h>
#include <memory>

#include <mysql/mysql.h>

#include "mprmpr/base/macros.h"
#include "mprmpr/db/result_row.h"

namespace mprmpr {
namespace db {

class ResultImpl;

// 负责管理由mysqld 返回的SQL语句执行结果:
//   MYSQL_RES*
//   (affected_rows, insert_id)
class Result {
 public:
  class Iterator {
   public:
    Iterator();
    Iterator(std::shared_ptr<ResultImpl> result, size_t index);
    Iterator(const Iterator& other);

    Iterator& operator=(const Iterator& other) {
      if (this != &other) {
        result_ = other.result_;
        index_ = other.index_;
      }
      return *this;
    }

    Iterator& operator++() {
      ++index_;
      return *this;
    }

    Iterator operator++(int) {
      Iterator it(*this);
      ++index_;
      return it;
    }

    bool operator==(const Iterator& other) {
      if (result_ != other.result_) return false;
      if (!Valid() && !other.Valid()) return true;
      return index_ == other.index_;
    }

    bool operator!=(const Iterator& other) {
      return !(*this == other);
    }

    ResultRow operator*();
    std::unique_ptr<ResultRow> operator->();

   private:
    std::shared_ptr<ResultImpl> result_;
    size_t index_;
    bool Valid() const;
  };

  Result(MYSQL_RES* result);
  Result(std::shared_ptr<ResultImpl>&& impl);
  Result(size_t affected_rows, uint64_t insert_id);
  Result(std::nullptr_t result);
  Result(Result&& other);
  virtual ~Result();

  bool Valid() const;
  size_t affected_rows() const;
  uint64_t insert_id() const;
  size_t size() const;

  ResultRow operator[](size_t index);

  Iterator begin() const;
  Iterator end() const;

 private:
  std::shared_ptr<ResultImpl> result_;
  size_t affected_rows_;
  uint64_t insert_id_;

  DISALLOW_COPY_AND_ASSIGN(Result);
};

} // namespace db
} // namespace mprmpr

#endif // DB_RESULT_H_

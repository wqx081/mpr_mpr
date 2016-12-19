#ifndef MPRMPR_DB_RESULT_H_
#define MPRMPR_DB_RESULT_H_

#include <stdint.h>
#include <memory>

#include <mysql/mysql.h>

namespace mprmpr {
namespace db {

class ResultImpl;

class Result {
 public:
  class Iterator {
   public:
    Iterator();
    Iterator(std::shared_ptr<ResultImpl> result, size_t index);
    Iterator(const Iterator& other);
    Iterator& operator=(const Iterator& other);
    Iterator& operator++();
    Iterator& operator++(int offset);
    bool operator==(const Iterator& other);
    bool operator!=(const Iterator& other);
    // TODO(wqx)
    // ResultRow operator*()
    // std::unique_ptr<ResultRow> operator->();
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
  Result(const Result& other) = delete;
  virtual ~Result();
  bool Valid() const;
  size_t affected_rows() const;
  uint64_t insert_id() const;
  size_t size() const;

  // TODO(wqx)
  // ResultRow operator[](size_t index);

  Iterator begin() const;
  Iterator end() const;

 private:
  std::shared_ptr<ResultImpl> result_;
  size_t affected_rows_;
  uint64_t insert_id_;
};

} // namespace db
} // namespace mprmpr

#endif // DB_RESULT_H_

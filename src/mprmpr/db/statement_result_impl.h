#ifndef MPRMPR_DB_STATEMENT_RESULT_IMPL_H_
#define MPRMPR_DB_STATEMENT_RESULT_IMPL_H_

#include "mprmpr/db/result_impl.h"

namespace mprmpr {
namespace db {

class StatementResultImpl : public ResultImpl {
 public:
  StatementResultImpl(const std::map<std::string, size_t>& fields, 
                      std::vector<std::vector<std::unique_ptr<ResultFieldImpl>>>&& rows)
    : fields_(fields),
      rows_(std::move(rows)) {}

  virtual ~StatementResultImpl() {}

  
  const std::map<std::string, size_t>& Fields() const override {
    return fields_;
  }

  size_t size() const override {
    return rows_.size();
  }

  const std::vector<std::unique_ptr<ResultFieldImpl>>& Fetch(size_t index) override {
    return rows_[index];
  }

 private:
  std::map<std::string, size_t> fields_;
  std::vector<std::vector<std::unique_ptr<ResultFieldImpl>>> rows_;
};

} // namespace db
} // namespace mprmpr
#endif // MPRMPR_DB_STATEMENT_RESULT_IMPL_H_

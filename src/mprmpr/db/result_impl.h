#ifndef MPRMPR_DB_RESULT_IMPL_H_
#define MPRMPR_DB_RESULT_IMPL_H_
#include <memory>
#include <vector>
#include <map>

#include "mprmpr/db/result_field_impl.h"

namespace mprmpr {
namespace db {

class ResultImpl {
 public:
  virtual const std::map<std::string, size_t>& Fields() const = 0;
  virtual size_t size() const = 0;
  virtual const std::vector<std::unique_ptr<ResultFieldImpl>>& Fetch(size_t index) = 0;

  virtual ~ResultImpl() {};
};

} // namespace db
} // namespace mprmpr
#endif // MPRMPR_DB_RESULT_IMPL_H_

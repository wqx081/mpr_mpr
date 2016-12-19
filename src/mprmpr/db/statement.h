#ifndef MPRMPR_DB_STATEMENT_H_
#define MPRMPR_DB_STATEMENT_H_
#include <memory>
#include <mysql/mysql.h>

#include "mprmpr/base/macros.h"
#include "mprmpr/db/connection.h"
#include "mprmpr/util/status.h"

namespace mprmpr {
namespace db {

class StatementResultInfo;
class Result;

class Statement {
 public:
  Statement(Connection* conn, std::string statement);
  Statement(Statement&& other);
  virtual ~Statement();

  template<typename... Args>
  Status Execute(Result& result, Args...args) {
//    auto count = sizeof...(args);
//    auto* ps = new Parameter[sizeof...(args)]{args...};
//    return connection_->Execute(result, ps, count);
    return Status::OK();
  }

 private:
  Connection* connection_;
  MYSQL_STMT* statement_;
  size_t parameters_;
  std::unique_ptr<StatementResultInfo> info_;
  Status DoExecute(Result& result, Parameter* parameters, size_t count);

  DISALLOW_COPY_AND_ASSIGN(Statement);
};

} // namespace db
} // namespace mprmpr
#endif // MPRMPR_DB_STATEMENT_H_

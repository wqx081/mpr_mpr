#ifndef MPRMPR_DB_STATEMENT_RESULT_INFO_H_
#define MPRMPR_DB_STATEMENT_RESULT_INFO_H_
#include <vector>
#include <map>
#include <memory>
#include <mysql/mysql.h>

namespace mprmpr {
namespace db {

class StatementResultImpl;

class StatementResultInfo {
 public:
  StatementResultInfo(MYSQL_STMT* statement, MYSQL_RES* result);
  const std::map<std::string, size_t> Fields() const;
  size_t size() const;
  std::shared_ptr<StatementResultImpl> Rows();

 private:
  std::vector<MYSQL_BIND> bind_;
  MYSQL_STMT* statement_;
  std::map<std::string, size_t> fields_;
};

} // namespace db
} // namespace mprmpr
#endif // MPRMPR_DB_STATEMENT_RESULT_INFO_H_

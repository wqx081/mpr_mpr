#ifndef MPRMPR_DB_CONNECTION_H_
#define MPRMPR_DB_CONNECTION_H_

#include <unordered_map>
#include <memory>
#include <functional>

#include <mysql/mysql.h>

#include "mprmpr/base/macros.h"
#include "mprmpr/util/status.h"

namespace mprmpr {
namespace db {

class LocalParameter;
class Statement;
class Result;

class Connection {
  friend class Statement;
  friend class CachedStatement;

 public:
  Connection(const std::string& hostname, 
             const std::string& username, 
             const std::string& password,
             const std::string& database, uint64_t flags);

  virtual ~Connection();

  Status Connect();
  Status Query(Result& result, const std::string& query);

  template<typename... Args>
  Status Execute(Result& result, const std::string& query, Args... args) {
    if (sizeof...(args) == 0) {
      return this->Query(result, query);
    }
    std::string ret;
    Status s = Prepare(query, new LocalParameter[sizeof...(args)] { args...}, sizeof...(args), ret);
    if (!s.ok()) {
      return s;
    }
    return this->Query(result, ret);
  }

 private:
  MYSQL* connection_;
  std::unordered_map<const char*, std::unique_ptr<Statement>> statements_;
  std::string hostname_;
  std::string username_;
  std::string password_;
  std::string database_;
  uint64_t flags_;

  Statement* CreateStatement(const char* query);
  Status Prepare(const std::string& query, LocalParameter* parameters, size_t count, std::string& result);

  
  DISALLOW_COPY_AND_ASSIGN(Connection);
};

} // namespace db
} // namespace mprmpr
#endif // MPRMPR_DB_CONNECTION_H_

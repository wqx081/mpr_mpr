#include "mprmpr/db/connection.h"
#include "mprmpr/db/statement.h"
#include "mprmpr/db/local_parameter.h"
#include "mprmpr/db/result.h"

#include <glog/logging.h>

namespace mprmpr {
namespace db {

// 初始化Mysql library
namespace {

class MysqlLibrary {
 public:
  MysqlLibrary() {
    ::mysql_library_init(0, nullptr, nullptr);
  }    
  ~MysqlLibrary() {
    ::mysql_library_end();
  }
};

static MysqlLibrary mysql_library_init_;
} // namespace

///
ConnectionBuilder::ConnectionBuilder() {
  flags_ = CLIENT_IGNORE_SIGPIPE | CLIENT_MULTI_STATEMENTS;
}

ConnectionBuilder& ConnectionBuilder::set_hostname(const std::string& hostname) {
  hostname_ = hostname;
  return *this;
}

ConnectionBuilder& ConnectionBuilder::set_username(const std::string& username) {
  username_ = username;
  return *this;
}

ConnectionBuilder& ConnectionBuilder::set_password(const std::string& password) {
  password_ = password;
  return *this;
}

ConnectionBuilder& ConnectionBuilder::set_database(const std::string& database) {
  database_ = database;
  return *this;
}

ConnectionBuilder& ConnectionBuilder::set_flags(uint64_t flags) {
  flags_ = flags;
  return *this;
}

Status ConnectionBuilder::Build(std::unique_ptr<Connection>* conn) const {
  conn->reset(new Connection(hostname_, username_, password_, database_, flags_));
  RETURN_NOT_OK((*conn)->Connect());
  return Status::OK();
}

///
Connection::Connection(const std::string& hostname, const std::string& username,
                       const std::string& password, const std::string& database, uint64_t flags)
    : connection_(::mysql_init(nullptr)) ,
      hostname_(hostname), username_(username), password_(password), database_(database), 
      flags_(flags) {
  DCHECK(connection_);
}

Status Connection::Connect() {
  // enable auto reconnect
  // 如果执行某条语句失败是因为与server断了连接, 那么会自动重新与服务建立连接，并且再次执行已经失败的语句.
  my_bool reconnect = 1;
  ::mysql_options(connection_, MYSQL_OPT_RECONNECT, &reconnect);

  // MYSQL *mysql_real_connect(MYSQL *mysql, 
  //                           const char *host, 
  //                           const char *user, 
  //                           const char *passwd, 
  //                           const char *db, 
  //                           unsigned int port, 
  //                           const char *unix_socket, 
  //                           unsigned long client_flag)
  // client_flag:
  //    CLIENT_IGNORE_SIGPIPE   防止与应用程序同时注册 SIGPIPE handler冲突
  //    CLIENT_MULTI_RESULTS    允许处理多条语句执行或者存储过程返回多个结果集
  //                            如果此选项设置，那么 CLIENT_MULTI_STATEMENTS 也被隐式设置
  //                            
  //    CLIENT_MULTI_STATEMENTS 允许在一条字符串中包含多条SQL语句，语句之间只用 ';' 分隔
  if (::mysql_real_connect(connection_, hostname_.c_str(), username_.c_str(),
                           password_.c_str(), database_.c_str(), 0, nullptr, flags_) == nullptr) {
    return Status::InvalidArgument(::mysql_error(connection_));
  }
  return Status::OK();
}

Connection::~Connection() {
  if (connection_) {
    ::mysql_close(connection_);
  }
  ::mysql_thread_end();
}

// private
Statement* Connection::CreateStatement(const char* query) {
  auto iter = statements_.find(query);
  if (iter != statements_.end()) {
    return iter->second.get();
  }

  auto* statement = new Statement(this, query);
  statements_[query].reset(statement);

  return statement;
}

Status Connection::Prepare(const std::string& query,
                           LocalParameter* parameters,
                           size_t count,
                           std::string& result) {
  size_t sz = query.size();
  for (size_t i = 0; i < count; ++i) {
    sz += parameters[i].size();
  }
  result.reserve(sz);

  size_t position = query.find_first_of("?!");
  result.append(query, 0, position);

  for (size_t i = 0; i < count; ++i) {
    if (position == std::string::npos) {
      break;
    }
    auto& pa = parameters[i];
    switch (query[position]) {
      case '?': result.append(pa.Quote(connection_)); break;
      case '!': result.append(pa.Escape(connection_)); break;
    }

    size_t next = query.find_first_of("?!", position + 1);
    result.append(query, position + 1, next - position - 1);
    position = next;
  }

  delete [] parameters;
  return Status::OK();
}

Status Connection::Query(const std::string& query) {

  results_.clear();

  // 执行SQL语句
  // 如果语句中包含Binary data, 那么使用 mysql_real_query
  if (::mysql_query(connection_, query.c_str())) {
    return Status::RuntimeError(::mysql_error(connection_));
  }
  
  while (true) {
    // MYSQL_RES* mysql_store_result(MYSQL* mysql):
    //  Reads the entire result of a query to the client, allocates a MYSQL_RES structure, 
    //  and places the result into this structure.
    //  Returns a null pointer if the statement didn't return a result set(e.g., if it was an INSERT statement).
    //  Also returns a null pointer if reading of the result set failed.
    //
    // unsigned int mysql_field_count(MYSQL* mysql);
    //  Returns the number of columns for the most recent query on the connection.
    //
    // mysql_affected_rows() 
    //  Returns the number of rows changed/deleted/inserted by the last UPDATE, DELETE, or INSERT query.
    //
    // my_ulonglong mysql_insert_id(MYSQL* mysql)
    //  Returns the value generated for an AUTO_INCREMENT column by the previous INSERT or UPDATE
    //  statement. Otherwise, return 0.
    //
    MYSQL_RES* result = ::mysql_store_result(connection_);
    std::unique_ptr<Result> rt;

    if (result) { // SELECT,SHOW,DESCRIBE,EXPLAINT,CHECK TABLE...
      // 把结果 result 传入 Result 对象
      // 由Result 对象负责 mysql_free_result(result)
      rt.reset(new Result(result));
      results_.push_back(std::move(rt));
    } else if (::mysql_field_count(connection_)) {
      return Status::RuntimeError(::mysql_error(connection_));
    } else { // UPDATE, INSERT, DELETE
      size_t affected_rows = ::mysql_affected_rows(connection_);
      rt.reset(new Result(affected_rows,
                          ::mysql_insert_id(connection_)));
      results_.push_back(std::move(rt));
    }

    switch (::mysql_next_result(connection_)) {
      case -1: // NO more results
        return Status::OK();
      case 0:  // Yes, keep looping
        continue;
      default: // We encounter ERROR
        return Status::RuntimeError(::mysql_error(connection_));
    }
  }

  return Status::OK();
}

} // namespace db
} // namespace mprmpr

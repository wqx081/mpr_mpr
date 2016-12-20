#include "mprmpr/db/connection.h"
#include "mprmpr/db/statement.h"
#include "mprmpr/db/local_parameter.h"
#include "mprmpr/db/result.h"

#include <glog/logging.h>

namespace mprmpr {
namespace db {

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
  my_bool reconnect = 1;
  ::mysql_options(connection_, MYSQL_OPT_RECONNECT, &reconnect);

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

Status Connection::Query(std::vector<Result>& results, const std::string& query) {
  if (::mysql_query(connection_, query.c_str())) {
    return Status::RuntimeError(::mysql_error(connection_));
  }

  while (true) {
    auto* result = ::mysql_store_result(connection_);
    size_t affected_rows = ::mysql_affected_rows(connection_);
    if (result) {
      results.push_back(Result(result));
    } else if (::mysql_field_count(connection_)) {
      return Status::RuntimeError(::mysql_error(connection_));
    } else { // UPDATE, INSERT, DELETE
      results.push_back(Result(affected_rows,
                               ::mysql_insert_id(connection_)));
    }

    switch (::mysql_next_result(connection_)) {
      case -1: return Status::OK();
      case 0:  continue;
      default: return Status::RuntimeError(::mysql_error(connection_));
    }
  }

  return Status::OK();
}

} // namespace db
} // namespace mprmpr

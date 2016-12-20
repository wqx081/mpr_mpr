#include "mprmpr/db/statement.h"
#include "mprmpr/db/statement_result_impl.h"
#include "mprmpr/db/statement_result_info.h"
#include "mprmpr/db/result.h"

#include <mysql/errmsg.h>

#include <glog/logging.h>

namespace mprmpr {
namespace db {

Statement::Statement(Connection* connection, std::string statement)
    : connection_(connection),
      statement_(nullptr),
      query_(std::move(statement)),
      parameters_(0) {
  Init();      
}

Statement::Statement(Statement&& other)
    : connection_(other.connection_),
      statement_(other.statement_),
      query_(other.query_),
      parameters_(other.parameters_),
      info_(std::move(other.info_)) {
  other.connection_ = nullptr;
  other.statement_ = nullptr;
  other.parameters_ = 0;  
}

Statement::~Statement() {
  if (statement_) {
    ::mysql_stmt_close(statement_);
  }
}

void Statement::Init() {
  statement_ = ::mysql_stmt_init(connection_->connection_);
  DCHECK(statement_) << "Unable to init statement";   
  if (::mysql_stmt_prepare(statement_, query_.c_str(), query_.size())) {
    LOG(ERROR) << ::mysql_stmt_error(statement_);
    ::mysql_stmt_close(statement_);
    statement_ = nullptr;
    DCHECK(false);
  }
  parameters_ = ::mysql_stmt_param_count(statement_);
  auto* result = ::mysql_stmt_result_metadata(statement_);
  if (result != nullptr) {
    info_.reset(new StatementResultInfo(statement_, result));
  }
}

Status Statement::DoExecute(std::unique_ptr<Result>* result, Parameter* parameters, size_t count) {
  if (statement_ == nullptr) {
    delete[] parameters;
    return Status::InvalidArgument("Cannot exeucte invalid statement: ");
  }

  if (count != parameters_) {
    delete[] parameters;
    return Status::InvalidArgument("Incorrect number of arguments");
  }

  if (::mysql_stmt_bind_param(statement_, parameters)) {
    delete [] parameters;
    return Status::RuntimeError(::mysql_stmt_error(statement_));
  }

  if (::mysql_stmt_execute(statement_)) {
    if (::mysql_stmt_errno(statement_) == CR_SERVER_LOST) {
      statement_ = nullptr;
      parameters_ = 0;
      info_.reset();
      Init();
      return DoExecute(result, parameters, count);
    } else {
      delete [] parameters;
      return Status::RuntimeError(::mysql_stmt_error(statement_));
    }
  }

  delete [] parameters;

  if (!info_) {
    result->reset(new Result(::mysql_stmt_affected_rows(statement_), ::mysql_stmt_insert_id(statement_)));
  } else {
    auto rows = info_->Rows();
    result->reset(new Result(std::move(rows)));
  }
  return Status::OK();
}

} // namespace db
} // namespace mprmpr

#include <cstdlib>
#include <cstring>

#include <glog/logging.h>

#include "mprmpr/db/statement_result_info.h"
#include "mprmpr/db/statement_result_impl.h"
#include "mprmpr/db/statement_result_field.h"
#include "mprmpr/db/statement_integral_result_field.h"
#include "mprmpr/db/statement_dynamic_result_field.h"
#include "mprmpr/db/statement_datetime_result_field.h"

namespace mprmpr {
namespace db {

StatementResultInfo::StatementResultInfo(MYSQL_STMT* statement, MYSQL_RES* result)
    : statement_(statement) {
  size_t fields = ::mysql_num_fields(result);
  bind_.reserve(fields);

  while (auto* field = ::mysql_fetch_field(result)) {
    MYSQL_BIND bind;
    std::memset(&bind, 0, sizeof(bind));

    if (IS_NUM(field->type)) {
      bind.is_unsigned = field->flags & UNSIGNED_FLAG;
    }

    switch (field->type) {
      case MYSQL_TYPE_INT24:
        bind.buffer_type = MYSQL_TYPE_LONG;
        break;
      case MYSQL_TYPE_DECIMAL:
      case MYSQL_TYPE_NEWDECIMAL:
        bind.buffer_type = MYSQL_TYPE_STRING;
        break;
      case MYSQL_TYPE_ENUM:
      case MYSQL_TYPE_SET:
        bind.buffer_type = MYSQL_TYPE_STRING;
        break;
      case MYSQL_TYPE_GEOMETRY:
        bind.buffer_type = MYSQL_TYPE_BLOB;
        break;
      case MYSQL_TYPE_BIT:
        bind.buffer_type = MYSQL_TYPE_BLOB;
        break;
      default:
        bind.buffer_type = field->type;
        break;
    }
    fields_[std::string(field->name, field->name_length)] = bind_.size();
    bind_.push_back(bind);
  }  
  ::mysql_free_result(result);
}

const std::map<std::string, size_t> StatementResultInfo::Fields() const {
  return fields_;
}

size_t StatementResultInfo::size() const {
  return bind_.size();
}

std::shared_ptr<StatementResultImpl> StatementResultInfo::Rows() {
  if (::mysql_stmt_store_result(statement_)) {
    DCHECK(false) << mysql_stmt_error(statement_);
  }

  size_t count = ::mysql_stmt_num_rows(statement_);
  std::vector<std::vector<std::unique_ptr<ResultFieldImpl>>> result(count);

  for (auto& row : result) {

    row.reserve(bind_.size());

    for (auto& bind : bind_) {
      StatementResultField* field = nullptr;

      switch (bind.buffer_type) {
        case MYSQL_TYPE_TINY:
          field = new StatementSignedCharResultField();
          break;
        case MYSQL_TYPE_SHORT:
          if (bind.is_unsigned)   field = new StatementUnsignedShortResultField();
          else                    field = new StatementSignedShortResultField();
          break;
        case MYSQL_TYPE_INT24:
        case MYSQL_TYPE_LONG:
          if (bind.is_unsigned)   field = new StatementUnsignedLongResultField();
          else                    field = new StatementSignedLongResultField();
          break;
        case MYSQL_TYPE_LONGLONG:
          if (bind.is_unsigned)   field = new StatementUnsignedLongLongResultField();
          else                    field = new StatementSignedLongLongResultField();
          break;
        case MYSQL_TYPE_FLOAT:
          field = new StatementFloatResultField();
          break;
        case MYSQL_TYPE_DOUBLE:
          field = new StatementDoubleResultField();
          break;
        case MYSQL_TYPE_DECIMAL:
        case MYSQL_TYPE_NEWDECIMAL:
        case MYSQL_TYPE_ENUM:
        case MYSQL_TYPE_SET:
        case MYSQL_TYPE_GEOMETRY:
        case MYSQL_TYPE_BIT:
        case MYSQL_TYPE_VARCHAR:
        case MYSQL_TYPE_VAR_STRING:
        case MYSQL_TYPE_STRING:
        case MYSQL_TYPE_TINY_BLOB:
        case MYSQL_TYPE_MEDIUM_BLOB:
        case MYSQL_TYPE_LONG_BLOB:
        case MYSQL_TYPE_BLOB:
          field = new StatementDynamicResultField();
          break;
        case MYSQL_TYPE_YEAR:
        case MYSQL_TYPE_TIME:
        case MYSQL_TYPE_DATE:
        case MYSQL_TYPE_NEWDATE:
        case MYSQL_TYPE_DATETIME:
        case MYSQL_TYPE_TIMESTAMP:
          field = new StatementDatetimeResultField();
          break;
        case MYSQL_TYPE_NULL:
          break;
        default:
          break;
      } // switch

      if (field == nullptr) {
        row.emplace_back(nullptr);
        continue;
      }   

      if (!field->IsDynamic()) {
        bind.buffer = field->GetValue();
        bind.is_null = field->GetNull();
      } else {
        StatementDynamicResultField* dynamic = static_cast<StatementDynamicResultField*>(field);
        bind.buffer = nullptr;
        bind.is_null = field->GetNull();
        bind.length = &dynamic->size_;
        bind.buffer_length = 0;
      }

      // Add the field
      row.emplace_back(field);
    } // for 2
    
    if (::mysql_stmt_bind_result(statement_, bind_.data())) {
      DCHECK(false) << "Error: " << ::mysql_stmt_error(statement_);
    }
    
    switch (::mysql_stmt_fetch(statement_)) {
      case 0: 
        break;
      case 1: 
        DCHECK(false) << "Error: " << ::mysql_stmt_error(statement_); 
        break;
      case MYSQL_NO_DATA:
        DCHECK(false) << "Error: result set corrupted";
        break;
      case MYSQL_DATA_TRUNCATED:
        for (size_t i = 0; i < bind_.size(); ++i) {
          auto& bind = bind_[i];
          auto* field = static_cast<StatementResultField*>(row[i].get());
          if (!field->IsDynamic() || field->IsNULL()) {
            continue;
          }
          StatementDynamicResultField* dynamic = static_cast<StatementDynamicResultField*>(field);
          if (!dynamic->size_) {
            continue;
          }
          dynamic->value_ = static_cast<char*>(std::malloc(dynamic->size_));

          bind.buffer = dynamic->value_;
          bind.buffer_length = dynamic->size_;
 
          ::mysql_stmt_fetch_column(statement_, &bind, i, 0);
        }
        // done
        break;
    }
  } // for 1

  return std::make_shared<StatementResultImpl>(fields_, std::move(result));
}

} // namespace db
} // namespace mprmpr

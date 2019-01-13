/** Copyright 2018 He Hao<hehaoslj@sina.com> */

#include "lmapi.h"

#include "lmsql.h"

namespace lmapi {

/** sql_dataset constructor */
sql_dataset::sql_dataset(const std::string& conns, const std::string& query) {
  sql_internal* sql;
  int ret;

  sql = new sql_internal();
  pdata = sql;

  ret = sql->connect(conns);
  if (ret != 0) {
    return;
  }
  // printf("do query...\n");
  sql->select(query);
}

sql_dataset::~sql_dataset() {
  sql_internal* sql;

  sql = reinterpret_cast<sql_internal*>(pdata);
  if (sql) delete sql;
}

/** api */
const std::string& sql_dataset::get_error(void) const {
  const sql_internal* sql;

  sql = reinterpret_cast<const sql_internal*>(pdata);

  return sql->err_msg;
}

int sql_dataset::rows(void) const {
  const sql_internal* sql;

  sql = reinterpret_cast<const sql_internal*>(pdata);
  return sql->row_count;
}

int sql_dataset::cols(void) const {
  const sql_internal* sql;

  sql = reinterpret_cast<const sql_internal*>(pdata);
  return sql->col_count;
}

lmapi_sql_type sql_dataset::type(int col) const {
  const sql_internal* sql;
  lmapi_sql_type tp = LMAPI_SQL_UNKNOWN;

  sql = reinterpret_cast<const sql_internal*>(pdata);
  if (col < sql->col_count) {
    const sql_column& desc = sql->col_descs[col];
    switch (desc.var_dbtype) {
      case otl_var_bigint:
      case otl_var_long_int:
      case otl_var_ubigint:
        tp = LMAPI_SQL_INT64;
        break;
      case otl_var_char:
      case otl_var_varchar_long:
      case otl_var_long_string:
        tp = LMAPI_SQL_STRING;
        break;
      case otl_var_double:
        tp = LMAPI_SQL_FLOAT64;
        break;
      case otl_var_float:
        tp = LMAPI_SQL_FLOAT32;
        break;
      case otl_var_int:
      case otl_var_unsigned_int:
        tp = LMAPI_SQL_INT32;
        break;
      case otl_var_short:
        tp = LMAPI_SQL_INT16;
        break;
      case otl_var_timestamp:
        tp = LMAPI_SQL_DATETIME;
        break;
      case otl_var_raw_long:
      case otl_var_raw:
      case otl_var_ltz_timestamp:
      case otl_var_blob:
      case otl_var_clob:
      case otl_var_db2date:
      case otl_var_db2time:
        break;
      default:
        break;
    }
  }
  return tp;
}

int sql_dataset::get_int(int row, int col) const {
  const sql_internal* sql;
  size_t pos;
  int value = 0;

  sql = reinterpret_cast<const sql_internal*>(pdata);
  pos = row * sql->col_count + col;
  if (pos < sql->dataset.size()) {
    const sql_variable& var = sql->dataset[pos];
    if (var.type == otl_var_float)
      value = var.tdata.f32_var;
    else if (var.type == otl_var_double)
      value = var.tdata.f64_var;
    else
      value = var.tdata.i32_var;
  }
  return value;
}

int64_t sql_dataset::get_int64(int row, int col) const {
  const sql_internal* sql;
  size_t pos;
  int64_t value = 0;

  sql = reinterpret_cast<const sql_internal*>(pdata);
  pos = row * sql->col_count + col;
  if (pos < sql->dataset.size()) {
    const sql_variable& var = sql->dataset[pos];
    if (var.type == otl_var_float)
      value = var.tdata.f32_var;
    else if (var.type == otl_var_double)
      value = var.tdata.f64_var;
    else
      value = var.tdata.i64_var;
  }
  return value;
}

double sql_dataset::get_float64(int row, int col) const {
  const sql_internal* sql;
  size_t pos;
  double value = 0;

  sql = reinterpret_cast<const sql_internal*>(pdata);
  pos = row * sql->col_count + col;
  if (pos < sql->dataset.size()) {
    const sql_variable& var = sql->dataset[pos];
    if (var.type == otl_var_float)
      value = var.tdata.f32_var;
    else if (var.type == otl_var_double)
      value = var.tdata.f64_var;
    else
      value = var.tdata.i64_var;
  }
  return value;
}
std::string sql_dataset::get_string(int row, int col) const {
  const sql_internal* sql;
  size_t pos;
  std::string value;
  std::ostringstream os;

  sql = reinterpret_cast<const sql_internal*>(pdata);
  pos = row * sql->col_count + col;
  if (pos < sql->dataset.size()) {
    const sql_variable& var = sql->dataset[pos];
    if (var.type == otl_var_char || var.type == otl_var_varchar_long ||
        var.type == otl_var_long_string)
      value = var.str_var;
    else if (var.type == otl_var_double) {
      os << var.tdata.f64_var;
      value = os.str();
    } else if (var.type == otl_var_float) {
      os << var.tdata.f32_var;
      value = os.str();
    } else if (var.type == otl_var_timestamp) {
      os << var.tdata.u64_var;
      value = os.str();
    } else {
      os << var.tdata.i64_var;
      value = os.str();
    }
  }
  return value;
}

}  // namespace lmapi

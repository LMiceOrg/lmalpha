/** Copyright 2018, 2019 He Hao<hehaoslj@sina.com> */
#ifndef LMAPI_LMSQL_H_
#define LMAPI_LMSQL_H_

#include <iconv.h>
#include <stdint.h>

#include <sstream>
#include <string>
#include <vector>

#include "lmapi.h"

class otl_connect;

namespace lmapi {

union trival_data {
  int16_t i16_var;
  int32_t i32_var;
  int64_t i64_var;
  uint16_t u16_var;
  uint32_t u32_var;
  uint64_t u64_var;
  float f32_var;
  double f64_var;
};
struct sql_variable {
  int type;
  int lmtype;
  trival_data tdata;
  std::string str_var;
};
struct sql_column {
  char name[32];
  int var_dbtype;
  int lmtype;
};

struct sql_internal {
  otl_connect* db;  // connect object
  int row_count;
  int col_count;
  std::string err_msg;
  std::vector<sql_column> col_descs;
  std::vector<sql_variable> dataset;
  iconv_t codec;
  iconv_t sqlstr_codec;

  sql_internal();

  ~sql_internal();

  /** helper */
  int connect(const std::string& conn_string);

  void disconnect(void);

  void select(const std::string& query);

  void insert(const std::string& query);

  void execute(const std::string& query);

  void insert(const std::string& format, const std::string& f1,
              const std::string& f2, const std::vector<lmapi_result_data>& rd);
};

}  // namespace lmapi

#endif /** LMAPI_LMSQL_H_ */

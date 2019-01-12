/** Copyright 2018 He Hao<hehaoslj@sina.com> */
#include <algorithm>
#include <functional>

#include "lmapi.h"

#include <call_thunk.h>

#if defined(_WIN32)

#else

#endif

/** c++ part */

namespace lmapi {
/** lmapi */
lmapi::lmapi() { /** lmapi configuration and initialization */
}
lmapi::~lmapi() {}

/** config */
config* lmapi::config_open(const std::string& name) {
  config* cfg = new config(name);
  return cfg;
}

void lmapi::config_close(config* cfg) { delete cfg; }

/** console log */
console* lmapi::console_open(int tp) {
  console* con = new console(tp);
  return con;
}

void lmapi::console_close(console* con) { delete con; }

/** sql query */

//      "Driver=SQLite3;Database=/Users/hehao/work/"
//      "build-alpha-Desktop_Qt_5_10_1_clang_64bit-Debug/test.db";
#define LMAPI_DSN                           \
  "Driver={ODBC Driver 17 for SQL Server};" \
  "Server=192.168.2.106;"                   \
  "UId=jydb;Pwd=jydb;"                      \
  "Connect Timeout=2;";

sql_dataset* lmapi::sql_open(const std::string& query) {
  std::string dsn;
  dsn = LMAPI_DSN;
  sql_dataset* ds = new sql_dataset(dsn, query);
  return ds;
}

void lmapi::sql_close(sql_dataset* ds) { delete ds; }

/** result store */
factor_result* lmapi::result_open(const std::string& factor_name) {
  std::string dsn = LMAPI_DSN;
  factor_result* ds = new factor_result(dsn, factor_name);
  return ds;
}

void lmapi::result_close(factor_result* ds) { delete ds; }

}  // namespace lmapi

/* c part */

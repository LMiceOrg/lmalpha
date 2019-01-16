/** Copyright 2018 He Hao<hehaoslj@sina.com> */
#include <string.h>

#include <algorithm>
#include <functional>
#include <string>

#include "lmapi.h"
#include "lmstock.h"

#include <call_thunk.h>

/** xml2 */
#include <libxml/parser.h>
#include <libxml/xmlmemory.h>

#if defined(WIN32) || defined(WIN64)
#define WIN32_MEAN_AND_LEAN
#include <Windows.h>

#elif defined(__APPLE__)
#include <mach-o/dyld.h>

#endif

#define LMAPI_CONFIG_NAME "lmalpha"

static void GetExectuableFileName(char* buf, unsigned int* len) {
#if defined(WIN32) || defined(WIN64)
  GetModuleFileNameA(NULL, buf, *len);

#elif defined(__APPLE__)

  _NSGetExecutablePath(buf, len);


#elif defined(UNIX)
  if (readlink("/proc/self/exe", buf, *len) == -1) path = buf;

#elif defined(__FreeBSD__)
  int mib[4];
  mib[0] = CTL_KERN;
  mib[1] = KERN_PROC;
  mib[2] = KERN_PROC_PATHNAME;
  mib[3] = -1;
  sysctl(mib, 4, buf, sizeof(buf), NULL, 0);
  path = buf;

#elif defined(SUNOS)
  buf = getexecname();
#endif
}

/** c++ part */

namespace lmapi {

struct lmapi_internal {
  std::string rtick_root;
  std::string kdata_root;
  std::string sql_dsn;
};

/** lmapi */
lmapi::lmapi() {
  std::string cfg_file;
  char path[1024];
  unsigned int len = 1024;

  lmapi_internal* api = new lmapi_internal;

  /** 1. Get app path */
  memset(path, 0, sizeof(path));
  GetExectuableFileName(path, &len);

  std::string str = path;
  size_t pos = str.rfind("/", str.size());
  if (pos > 0) {
    str = str.substr(0, pos + 1) + LMAPI_CONFIG_NAME;
  } else {
    str += LMAPI_CONFIG_NAME;
  }
  // printf("exe path:%s pos:%lu\n", str.c_str(), pos);

  /** 2. get xml config */
  cfg_file = str + ".xml";
  config* cfg = config_open(cfg_file);
  if (cfg->is_open()) {
    api->rtick_root = cfg->get_string("root.rawtick.rootPath");
    api->kdata_root = cfg->get_string("root.KData.rootPath");
    api->sql_dsn = cfg->get_string("root.SqlDSN");
  }
  delete cfg;

  cfg_file = str + ".json";
  cfg = config_open(cfg_file);
  if (cfg->is_open()) {
    if (api->rtick_root.empty())
      api->rtick_root = cfg->get_string("root.rawtick.rootPath");
    if (api->kdata_root.empty())
      api->kdata_root = cfg->get_string("root.KData.rootPath");
    if (api->sql_dsn.empty()) api->sql_dsn = cfg->get_string("root.SqlDSN");
  }
  delete cfg;

  pdata = api;
}

lmapi::~lmapi() {
  lmapi_internal* api;
  api = reinterpret_cast<lmapi_internal*>(pdata);
  delete api;
}

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
  "Uid=alpha_master;Pwd=alpha_master;"

//"Connect Timeout=60;";

sql_dataset* lmapi::sql_open(const std::string& query) {
  std::string dsn;
  lmapi_internal* api;

  api = reinterpret_cast<lmapi_internal*>(pdata);
  dsn = api->sql_dsn;
  sql_dataset* ds = new sql_dataset(dsn, query);
  // printf("query:%s\n", query.c_str());
  return ds;
}

void lmapi::sql_close(sql_dataset* ds) { delete ds; }

/** result store */
factor_result* lmapi::result_open(const std::string& factor_name) {
  std::string dsn;
  lmapi_internal* api;

  api = reinterpret_cast<lmapi_internal*>(pdata);
  dsn = api->sql_dsn;
  factor_result* ds = new factor_result(dsn, factor_name);
  return ds;
}

void lmapi::result_close(factor_result* ds) { delete ds; }

/** data load */
serial_dataset* lmapi::serial_open(const std::vector<std::string>& instrument,
                                   int tp, int start_date, int end_date) {
  lmapi_internal* api;

  api = reinterpret_cast<lmapi_internal*>(pdata);
  serial_dataset* ds;
  if (tp == LMAPI_TICK_TYPE) {
    ds = new serial_dataset(api->rtick_root, instrument, tp, start_date,
                            end_date);
  } else {
    ds = new serial_dataset(api->kdata_root, instrument, tp, start_date,
                            end_date);
  }
  return ds;
}

void lmapi::serial_close(serial_dataset* ds) { delete ds; }

}  // namespace lmapi

/* c part */

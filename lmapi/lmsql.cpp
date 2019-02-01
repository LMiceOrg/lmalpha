/** Copyright 2018, 2019 He Hao<hehaoslj@sina.com> */

#include <stdint.h>
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <cstdarg>

using std::wstring;

#define OTL_UNICODE  // Enable Unicode OTL for ODBC
#define OTL_CPP_11_ON
#define OTL_STL
#define OTL_ANSI_CPP

// Enable OTL Unicode rlogon and otl_exception for ODBC
#define OTL_UNICODE_EXCEPTION_AND_RLOGON

#if defined(__GNUC__)
#define OTL_UNICODE_CHAR_TYPE SQLWCHAR
#else
#define OTL_UNICODE_CHAR_TYPE wchar_t
#endif

// OTL support bigint
#define OTL_BIGINT int64_t

#if defined(_WIN32)
#define OTL_ODBC_MSSQL_2008  // Compile OTL 4/ODBC, MS SQL 2008
//#define OTL_ODBC // Compile OTL 4/ODBC. Uncomment this when used with MS
// SQL 7.0/ 2000
#else
#define OTL_ODBC_UNIX  // uncomment this line if UnixODBC is used
#endif

#include <otlv4.h>

#include "lmsql.h"

#include "lmapi.h"
#include "lmstrencode.h"

//#if defined(_WIN32)
//#define WIN32_MEAN_AND_LEAN
//#include <Windows.h>
//
//int togbk(const wchar_t* in, char* out, int out_len) {
//  return WideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS, in, -1, out, out_len,
//                             NULL, FALSE);
//}
//
//#endif

namespace lmapi {

/** helper */
static inline size_t utf16_size(const unsigned char* str) {
  size_t pos = 0;
  const uint16_t* p = reinterpret_cast<const uint16_t*>(str);
  do {
    if (*p != 0) {
      p++;
      pos++;
    }
  } while (*p != 0);

  return pos;
}

static inline size_t utf16_size(const SQLWCHAR* str) {
  const SQLWCHAR* p = str;
  size_t pos = 0;
  do {
    if (*p != 0) {
      p++;
      pos++;
    }
  } while (*p != 0);

  return pos;
}

static inline std::string utf16_utf8(iconv_t codec, const unsigned char* str) {
    const char* from = reinterpret_cast<const char*>(str);
    size_t from_bytes = utf16_size(str)*sizeof(uint16_t)+2;
    char * to = nullptr;
    size_t to_bytes = 0;
    std::string utf8_string;

    lmapi_strencode_capi()->utf16_to_utf8(from, from_bytes, &to, &to_bytes);
    utf8_string.insert(utf8_string.begin(), to, to + to_bytes);
    free(to);

  return utf8_string;
}

static inline std::string utf16_utf8(iconv_t codec, const SQLWCHAR* str) {
    const char* from = reinterpret_cast<const char*>(str);
    size_t from_bytes = utf16_size(str) * sizeof(uint16_t) + 2;
    char * to = nullptr;
    size_t to_bytes = 0;
    std::string utf8_string;

    lmapi_strencode_capi()->utf16_to_utf8(from, from_bytes, &to, &to_bytes);
    utf8_string.insert(utf8_string.begin(), to, to + to_bytes);
    free(to);

  return utf8_string;
}

static inline std::string exception_utf8(iconv_t codec,
                                         const otl_exception& e) {
  std::ostringstream oss;

  oss << utf16_utf8(codec, e.msg) << "|" << e.stm_text << "|"
      << utf16_utf8(codec, e.sqlstate) << "|" << e.var_info << "\n";

  return oss.str();
}

static inline std::string exception_utf8(const otl_exception& e) {
    std::string str;
    const char* from =nullptr;
    char* to = nullptr;
    size_t from_bytes = 0;
    size_t to_bytes = 0;
    auto strapi = lmapi_strencode_capi();
#if _WIN32
    auto cvt_locale = strapi->wstr_to_gbk;
#else
    auto cvt_locale = strapi->utf16_to_utf8;
#endif // _WIN32


    /** reserve 128 bytes */
    str.reserve(128);

    from = reinterpret_cast<const char*>(e.msg);
    from_bytes = utf16_size(e.msg)*sizeof(uint16_t)+2;
    cvt_locale(from, from_bytes, &to, &to_bytes);
    if(to)
        str += to;
    str += "|";
    str += e.stm_text;
    str += "|";

    from = reinterpret_cast<const char*>(e.sqlstate);
    from_bytes = utf16_size(e.sqlstate)*sizeof(uint16_t)+2;
    cvt_locale(from, from_bytes, &to, &to_bytes);
    if(to)
        str += to;
    str += "|";
    str += e.var_info;
    // str += "\n";

    /** clean encoding cache */
    if(to)
        free(to);

    return str;
    
}

static inline std::string utf8_utf16(iconv_t codec, const char* str) {
    std::string retstr;
    char* to_str = nullptr;
    size_t to_bytes = 0;
    const struct lmapi_strencode_api* api =lmapi_strencode_capi();
    api->utf8_to_wstr(str, strlen(str) + 1, &to_str, &to_bytes);
    if (to_str) {
        retstr.insert(retstr.begin(), to_str, to_str + to_bytes);
        free(to_str);
    }
  return retstr;
}

static inline sql_variable read_var(otl_stream& os, iconv_t codecs,
                                    const sql_column& column);

static inline int db_lmtype(int dbtype);

sql_internal::sql_internal() {
  row_count = 0;
  col_count = 0;
  db = nullptr;
#if defined(_WIN32)
  codec = iconv_open("GBK", "UTF-16LE");
#else
  codec = iconv_open("UTF-8", "UTF-16LE");
#endif

  sqlstr_codec = iconv_open("UTF-16LE", "UTF-8");
}

sql_internal::~sql_internal() {
  disconnect();
  iconv_close(codec);
  iconv_close(sqlstr_codec);
}

int sql_internal::connect(const std::string& conn_string) {
  // 1. initialize ODBC environment
  otl_connect::otl_initialize();

  // 2. login
  try {
    // printf("db conn...\n");
    db = new otl_connect();
    db->set_timeout(2);
    db->rlogon(conn_string.c_str());

    // set maximum long string size for connect object
    db->set_max_long_size(1024);
    return 0;

  } catch (otl_exception& e) {
    // intercept OTL exceptions

    err_msg = exception_utf8(e);
#if defined(_MSC_VER)
    printf("sql connect error %s\n", err_msg.c_str());

#endif
    // printf("sql conn failed %ls\n", e.msg);

    return 1;
  }
}

void sql_internal::disconnect(void) {
  try {
    // disconnect from the database
    db->logoff();
  } catch (otl_exception& e) {
    // intercept OTL exceptions
    err_msg = exception_utf8(e);
  }
}

void sql_internal::execute(const std::string& query) {
  try {
    //  1. execute
    otl_stream os(1, query.c_str(), *db);
  } catch (otl_exception& e) {
    // intercept OTL exceptions
    err_msg = exception_utf8(e);
  }
}
void sql_internal::execute_utf8str(const std::string& query,int size, ...) {
    try {
        
        char* to = nullptr;
        size_t to_bytes = 0;
        const struct lmapi_strencode_api* api = lmapi_strencode_capi();
        
        otl_stream os(1, query.c_str(), *db);
        std::va_list va;
        va_start(va, size);
        for (int i = 0; i < size; ++i) {
            const char* param = va_arg(va, const char*);
            api->utf8_to_wstr(param, strlen(param) + 1, &to, &to_bytes);
            const wchar_t *out_str = reinterpret_cast<const wchar_t*>(to);
            os << out_str;
        }
        va_end(va);
        free(to);
    } catch(otl_exception& e){ 
        err_msg = exception_utf8(e);
    }
}

void sql_internal::select(const std::string& query) {
  int array_size = 64;
  otl_column_desc* descs;
  size_t col_pos = 0;
  // otl_long_string ostr(1024);

  try {
    //  1. open
    otl_stream os(array_size, query.c_str(), *db);

    //  2. column desc
    descs = os.describe_select(col_count);
    if (col_count == 0) return;

    // printf("descv[0] %d %d\n", descs[0].dbtype, descs[0].otl_var_dbtype);

    col_descs.clear();
    col_descs.resize(col_count);
    memset(&col_descs[0], 0, sizeof(sql_column) * col_count);
    for (int i = 0; i < col_count; ++i) {
      const char* name = (descs + i)->name;
      size_t name_len = strlen(name);

      col_descs[i].var_dbtype = (descs + i)->otl_var_dbtype;
      col_descs[i].lmtype = db_lmtype(col_descs[i].var_dbtype);
      memcpy(col_descs[i].name, name,
             name_len > sizeof(sql_column::name) ? sizeof(sql_column::name) - 1
                                                 : name_len);
    }

    //  3. read dataset
    // printf("read dataset\n");
    dataset.clear();
    dataset.reserve(os.get_prefetched_row_count() * col_count);
    col_pos = 0;
    while (!os.eof()) {
      const sql_column& column = col_descs[col_pos % col_count];
      sql_variable var;

      var = read_var(os, codec, column);
      dataset.push_back(var);
      col_pos++;
    }

    //  4. rows
    row_count = dataset.size() / col_count;

    //  5. close stream
    os.close();

  } catch (otl_exception& e) {
    // intercept OTL exceptions
    // printf("readvar error\n" );
    err_msg = exception_utf8(e);
  }
}

void sql_internal::insert(const std::string& query) {
  try {
    otl_stream os(1, query.c_str(), *db);
  } catch (otl_exception& e) {
    // intercept OTL exceptions
    err_msg = exception_utf8(e);
  }
}

/** 特殊insert函数,插入计算结果 */
void sql_internal::insert(const std::string& format, const std::string& f1,
                          const std::string& f2,
                          const std::vector<lmapi_result_data>& rd) {
  // std::string uf1;
  // std::string uf2;
  auto strapi = lmapi_strencode_capi();
  const char* from = nullptr;
  char* to = nullptr;
  size_t from_bytes = 0;
  size_t to_bytes = 0;

  SQLWCHAR* uf1 = nullptr;
  SQLWCHAR* uf2 = nullptr;

  from = f1.c_str();
  from_bytes = f1.size()+1;
  to = nullptr;
  to_bytes = 0;
  strapi->utf8_to_utf16(from, from_bytes, &to, &to_bytes);
  uf1 = reinterpret_cast<SQLWCHAR*>(to);
  
  from = f2.c_str();
  from_bytes = f2.size()+1;
  to = nullptr;
  to_bytes = 0;
  strapi->utf8_to_utf16(from, from_bytes, &to, &to_bytes);
  uf2 = reinterpret_cast<SQLWCHAR*>(to);

  // uf1 = utf8_utf16(sqlstr_codec, f1.c_str());
  // uf2 = utf8_utf16(sqlstr_codec, f2.c_str());
  try {
      //printf("f1 %s f2 %s %lu\n", f1.c_str(), f2.c_str(), rd.size());
      
    otl_stream os(128, format.c_str(), *db);
    for (const auto& data : rd) {
      os << uf1
         << uf2 << data.date
         << data.time << data.value;
    }
  } catch (otl_exception& e) {
    // intercept OTL exceptions
    err_msg = exception_utf8(codec, e);
  }

  // Clean str buffer
  if(uf1)
  free(uf1);
  if(uf2)
  free(uf2);
}

static inline int db_lmtype(int dbtype) {
  int lmtype = LMAPI_SQL_UNKNOWN;
  switch (dbtype) {
    case otl_var_char:
    case otl_var_varchar_long:
    case otl_var_long_string:
      lmtype = LMAPI_SQL_STRING;
      break;
    case otl_var_double:
      lmtype = LMAPI_SQL_FLOAT64;
      break;
    case otl_var_float:
      lmtype = LMAPI_SQL_FLOAT32;
      break;

    case otl_var_short:
      lmtype = LMAPI_SQL_INT16;
      break;
    case otl_var_int:
    case otl_var_unsigned_int:
      lmtype = LMAPI_SQL_INT32;
      break;
    case otl_var_bigint:
    case otl_var_long_int:
    case otl_var_ubigint:
    case otl_var_timestamp:
    case otl_var_ltz_timestamp:
      lmtype = LMAPI_SQL_INT64;
      break;
    case otl_var_raw_long:
    case otl_var_raw:
    case otl_var_blob:
    case otl_var_clob:
    case otl_var_db2date:
    case otl_var_db2time:
    default:
      lmtype = LMAPI_SQL_UNKNOWN;
      break;
  }
  return lmtype;
}

static inline sql_variable read_var(otl_stream& os, iconv_t codecs,
                                    const sql_column& column) {
  sql_variable var;
  otl_datetime dt;
  otl_long_string ustr;

  var.type = column.var_dbtype;
  // printf("%s\t%d\n", column.name, column.var_dbtype);
  switch (column.var_dbtype) {
    case otl_var_bigint:
      os >> var.tdata.i64_var;
      var.lmtype = LMAPI_SQL_INT64;
      break;
    case otl_var_char:
    case otl_var_varchar_long:
    case otl_var_long_string:
      os >> ustr.v;
      var.str_var = utf16_utf8(codecs, ustr.v);
      var.lmtype = LMAPI_SQL_STRING;
      break;
    case otl_var_double:
      os >> var.tdata.f64_var;
      var.lmtype = LMAPI_SQL_FLOAT64;
      break;
    case otl_var_float:
      os >> var.tdata.f32_var;
      var.lmtype = LMAPI_SQL_FLOAT32;
      break;
    case otl_var_int:
      os >> var.tdata.i32_var;
      var.lmtype = LMAPI_SQL_INT32;
      break;
    case otl_var_long_int:
      os >> var.tdata.i64_var;
      var.lmtype = LMAPI_SQL_INT64;
      break;
    case otl_var_short:
      os >> var.tdata.i16_var;
      var.lmtype = LMAPI_SQL_INT16;
      break;
    case otl_var_ubigint:
      os >> var.tdata.i64_var;
      var.lmtype = LMAPI_SQL_INT64;
      break;
    case otl_var_unsigned_int:
      os >> var.tdata.u32_var;
      var.lmtype = LMAPI_SQL_INT32;
      break;
    case otl_var_timestamp:
    case otl_var_ltz_timestamp:
      os >> dt;
      //  yyyymmddhhmmss
      var.tdata.u64_var = dt.fraction / 1000000LL + dt.second * 1000LL +
                          dt.minute * 100000LL + dt.hour * 10000000LL +
                          dt.day * 1000000000LL + dt.month * 100000000000LL +
                          dt.year * 10000000000000LL;
      //        printf("%04d%02d%02d%02d%02d%02d%03llu\n%llu\n", dt.year,
      //        dt.month,
      //               dt.day, dt.hour, dt.minute, dt.second, dt.fraction /
      //               1000000LL, var.tdata.u64_var);
      var.type = otl_var_ubigint;
      var.lmtype = LMAPI_SQL_INT64;

      // var.tdata.u64_var = *(uint64_t*)(ostr->v);
      // printf("timestamp: %ld %d\n", dt.fraction, dt.frac_precision);
      break;
    case otl_var_raw_long:
    case otl_var_raw:

    case otl_var_blob:
    case otl_var_clob:
    case otl_var_db2date:
    case otl_var_db2time:
    default:
      os >> ustr;
      var.lmtype = LMAPI_SQL_UNKNOWN;
      break;
  }

  return var;
}

}  // namespace lmapi

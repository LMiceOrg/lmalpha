#include "lmsql.h"

#include <stdio.h>
#include <iostream>
#include <sstream>
#include <vector>

#if defined(_WIN32)
#define WIN32_MEAN_AND_LEAN
#include <Windows.h>

int togbk(const wchar_t* in, char* out, int out_len) {
  return WideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS, in, -1, out, out_len,
                             NULL, FALSE);
}

#endif

namespace lmapi {

/** helper */
static inline size_t utf16_size(const unsigned char* str) {
  size_t pos = 0;
  const unsigned short* p = reinterpret_cast<const unsigned short*>(str);
  do {
    if (*p != 0) {
      p++;
      pos++;
    }
  } while (*p != 0);

  return pos;
}

static inline size_t utf16_size(const SQLTCHAR* str) {
  const SQLTCHAR* p = str;
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
  std::ostringstream oss;
  char buff[1024];
  char* in;
  char* out;
  size_t in_size;
  size_t out_size = 1024;
  size_t ret;
  std::string utf8_string;

  in = reinterpret_cast<char*>(const_cast<unsigned char*>(str));
  in_size = utf16_size(str);
  if (in_size < 256) {
    in_size *= 2;
    memset(buff, 0, 1024);
    out = buff;
    ret = iconv(codec, &in, &in_size, &out, &out_size);
    utf8_string = buff;
  } else {
    out_size = in_size * 4;
    in_size *= 2;
    char* large_buff = (char*)malloc(out_size);
    memset(large_buff, 0, out_size);
    out = buff;
    utf8_string = large_buff;
    free(large_buff);
  }

  return utf8_string;
}

static inline std::string utf16_utf8(iconv_t codec, const SQLTCHAR* str) {
  std::ostringstream oss;
  char buff[1024];
  char* in;
  char* out;
  size_t in_size;
  size_t out_size = 1024;
  size_t ret;
  std::string utf8_string;

  in = reinterpret_cast<char*>(const_cast<SQLTCHAR*>(str));
  in_size = utf16_size(str);
  if (in_size < 256) {
    memset(buff, 0, 1024);
    out = buff;
    in_size *= 2;
    ret = iconv(codec, &in, &in_size, &out, &out_size);
    utf8_string = buff;
  } else {
    out_size = in_size * 4;
    in_size *= 2;
    char* large_buff = (char*)malloc(out_size);
    memset(large_buff, 0, out_size);
    out = buff;
    utf8_string = large_buff;
    free(large_buff);
  }

  return utf8_string;
}

static inline std::string exception_utf8(iconv_t codec,
                                         const otl_exception& e) {
  std::ostringstream oss;

  oss << utf16_utf8(codec, e.msg) << "|" << e.stm_text << "|"
      << utf16_utf8(codec, e.sqlstate) << "|" << e.var_info << "\n";

  return oss.str();
}

sql_internal::sql_internal() {
  row_count = 0;
  col_count = 0;
#if defined(_WIN32)
  codec = iconv_open("GBK", "UTF-16LE");
#else
  codec = iconv_open("UTF-8", "UTF-16LE");
#endif
}

sql_internal::~sql_internal() { disconnect(); }

int sql_internal::connect(const std::string& conn_string) {
  // 1. initialize ODBC environment
  otl_connect::otl_initialize();

  // 2. login
  try {
    // printf("db conn...\n");
    //    std::vector<wchar_t> wcstring(conn_string.size() + 1, 0);
    //    char* in = const_cast<char*>(conn_string.c_str());
    //    char* out = reinterpret_cast<char*>(&wcstring[0]);
    //    size_t in_size = conn_string.size();
    //    size_t out_size = (wcstring.size() - 1) * sizeof(wchar_t);
    //    iconv(codec, &in, &in_size, &out, &out_size);
    db.set_timeout(2);
    db.rlogon(conn_string.c_str());
    // printf("db conned...\n");WC_NO_BEST_FIT_CHARS
    return 0;

  } catch (otl_exception& e) {
    // intercept OTL exceptions
    err_msg = exception_utf8(codec, e);

    return 1;
  }
}

void sql_internal::disconnect(void) {
  try {
    // disconnect from the database
    db.logoff();
  } catch (otl_exception& e) {
    // intercept OTL exceptions
    err_msg = exception_utf8(codec, e);
  }
}

void sql_internal::select(const std::string& query) {
  int array_size = 64;
  otl_column_desc* descs;
  size_t col_pos = 0;
  otl_long_string ostr(1024);

  try {
    // set maximum long string size for connect object
    db.set_max_long_size(1024);

    //  1. open
    os.open(array_size, query.c_str(), db);

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

      var = read_var(os, column, &ostr);
      dataset.push_back(var);
      col_pos++;
    }

    //  4. rows
    row_count = dataset.size() / col_count;

    //  5. close stream
    os.close();

  } catch (otl_exception& e) {
    // intercept OTL exceptions
    err_msg = exception_utf8(codec, e);
  }
}

void sql_internal::insert(const std::string& query) {
  os.open(1, query.c_str(), db);
  os.flush();
}

sql_variable sql_internal::read_var(otl_stream& os, const sql_column& column,
                                    otl_long_string* ostr) {
  sql_variable var;
  otl_datetime dt;
  otl_long_unicode_string ustr;

  var.type = column.var_dbtype;
  // printf("%s\t%d\n", column.name, column.var_dbtype);
  switch (column.var_dbtype) {
    case otl_var_bigint:
      os >> var.tdata.i64_var;
      break;
    case otl_var_char:
    case otl_var_varchar_long:
    case otl_var_long_string:
      os >> ustr;
      var.str_var = utf16_utf8(codec, ustr.v);
      break;
    case otl_var_double:
      os >> var.tdata.f64_var;
      break;
    case otl_var_float:
      os >> var.tdata.f32_var;
      break;
    case otl_var_int:
      os >> var.tdata.i32_var;
      break;
    case otl_var_long_int:
      os >> var.tdata.i64_var;
      break;
    case otl_var_short:
      os >> var.tdata.i16_var;
      break;
    case otl_var_ubigint:
      os >> var.tdata.u64_var;
      break;
    case otl_var_unsigned_int:
      os >> var.tdata.u32_var;
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
      break;
  }

  return var;
}

}  // namespace lmapi

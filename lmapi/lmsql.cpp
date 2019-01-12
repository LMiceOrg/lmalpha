#include "lmsql.h"
namespace lmapi {

sql_internal::sql_internal() {
  row_count = 0;
  col_count = 0;
}

sql_internal::~sql_internal() { disconnect(); }

int sql_internal::connect(const std::string& conn_string) {
  // 1. initialize ODBC environment
  otl_connect::otl_initialize();

  // 2. login
  try {
    // printf("db conn...\n");
    db.set_timeout(2);
    db.rlogon(conn_string.c_str());
    // printf("db conned...\n");
    return 0;

  } catch (otl_exception& e) {
    // intercept OTL exceptions
    std::ostringstream os;
    os << e.msg << "\t" << e.stm_text << "\t" << e.sqlstate << "\t"
       << e.var_info << std::endl;
    err_msg += os.str();
    return 1;
  }
}

void sql_internal::disconnect(void) {
  try {
    // disconnect from the database
    db.logoff();
  } catch (otl_exception& e) {
    // intercept OTL exceptions
    std::ostringstream os;
    os << e.msg << "\t" << e.stm_text << "\t" << e.sqlstate << "\t"
       << e.var_info << std::endl;
    err_msg += os.str();
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
    std::ostringstream os;
    os << e.msg << "\t" << e.stm_text << "\t" << e.sqlstate << "\t"
       << e.var_info << std::endl;
    err_msg += os.str();
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

  var.type = column.var_dbtype;
  // printf("%s\t%d\n", column.name, column.var_dbtype);
  switch (column.var_dbtype) {
    case otl_var_bigint:
      os >> var.tdata.i64_var;
      break;
    case otl_var_char:
    case otl_var_varchar_long:
    case otl_var_long_string:
      os >> var.str_var;
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
      os >> var.str_var;
      break;
  }

  return var;
}

}  // namespace lmapi

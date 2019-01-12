#include <stdint.h>
#include <sstream>
#include <string>
#include <vector>

#define OTL_ODBC  // Compile OTL 4.0/ODBC
#define OTL_STL
#define OTL_BIGINT int64_t
#if defined(_WIN32)
#else
#define OTL_ODBC_UNIX  // uncomment this line if UnixODBC is used
#endif
#include <otlv4.h>

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
  trival_data tdata;
  std::string str_var;
};
struct sql_column {
  char name[32];
  int var_dbtype;
};

struct sql_internal {
  otl_connect db;  // connect object
  otl_stream os;   // sql stream
  int row_count;
  int col_count;
  std::string err_msg;
  std::vector<sql_column> col_descs;
  std::vector<sql_variable> dataset;

  sql_internal();

  ~sql_internal();

  /** helper */
  int connect(const std::string& conn_string);

  void disconnect(void);

  sql_variable read_var(otl_stream& os, const sql_column& column,
                        otl_long_string* ostr);

  void select(const std::string& query);

  void insert(const std::string& query);
};

}  // namespace lmapi

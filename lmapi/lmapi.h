#ifndef LMAPI_LMAPI_H_
#define LMAPI_LMAPI_H_
/** Copyright 2018 He Hao<hehaoslj@sina.com> */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** c-api */

typedef void* lmapi_t;

lmapi_t lmapi_create(lmapi_t* api);
void lmapi_destroy(lmapi_t* api);

/** config read */
typedef void* lmapi_config_t;
struct lmapi_config_read {
  /** value */
  int (*get_int)(const char* name);
  int64_t (*get_int64)(const char* name);
  float (*get_float)(const char* name);
  double (*get_float64)(const char* name);
  const char* (*get_string)(const char* name);

  /** array */
  int (*get_array_size)(const char* name);
  int (*get_array_int)(const char* name, int idx);
  int64_t (*get_array_int64)(const char* name, int idx);
  float (*get_array_float)(const char* name, int idx);
  double (*get_array_double)(const char* name, int idx);
  const char* (*get_array_string)(const char* name, int idx);
};
lmapi_config_t* lmapi_config_open(lmapi_t api, const char* cfg_file);
void lmapi_config_close(lmapi_config_t* cfg);

/** console logging */
enum lmapi_console_type {
  LMAPI_LOG_ALL = 0,
  LMAPI_LOG_DATETIME = 1 << 0,
  LMAPI_LOG_PROCESS = 1 << 1,
  LMAPI_LOG_THREAD = 1 << 2
};
struct lmapi_console {
  void (*debug)(const char* format, ...);
  void (*info)(const char* format, ...);
  void (*warning)(const char* format, ...);
  void (*critical)(const char* format, ...);
  void (*error)(const char* format, ...);
};
struct lmapi_console* lmapi_console_open(lmapi_t api,
                                         enum lmapi_console_type tp);
void lmapi_console_close(lmapi_t api, struct lmapi_console* log);

/** data load */
enum lmapi_data_type {
  LMAPI_TICK_TYPE,
  LMAPI_30SEC_TYPE,
  LMAPI_1MIN_TYPE,
  LMAPI_5MIN_TYPE,
  LMAPI_15MIN_TYPE,
  LMAPI_30MIN_TYPE,
  LMAPI_1HOUR_TYPE,
  LMAPI_1DAY_TYPE,
  LMAPI_3DAY_TYPE,
  LMAPI_1WEEK_TYPE,
  LMAPI_1MONTH_TYPE
};
void* lmapi_data_load(lmapi_t api, enum lmapi_data_type tp,
                      const char* instrument, const char* start_date,
                      const char* end_date, int* size);

/** result store */
struct lmapi_result_info {
  char factor_name[64];
  char factor_author[32];
  char factor_date[32];
};
struct lmapi_result_data {
  int date;
  int time;
  double value;
};
struct lmapi_result_dataset {
  int size;
  int type;
  char instrument[32];

  lmapi_result_data data[1];
};
void lmapi_result_store_info(lmapi_t api, struct lmapi_result_info* info);
void lmapi_result_store_dataset(lmapi_t api, struct lmapi_result_dataset* ds);

/** sql querty */
enum lmapi_sql_type {
  LMAPI_SQL_UNKNOWN,
  LMAPI_SQL_INT16,
  LMAPI_SQL_INT32,
  LMAPI_SQL_INT64,
  LMAPI_SQL_FLOAT32,
  LMAPI_SQL_FLOAT64,
  LMAPI_SQL_DATETIME,
  LMAPI_SQL_STRING
};
struct lmapi_sql_dataset {
  int (*rows)(void);
  int (*cols)(void);
  enum lmapi_sql_type (*type)(int col);

  void* (*at)(int row, int col);

  int (*get_int)(int row, int col);
  int64_t (*get_int64)(int row, int col);
  double (*get_float)(int row, int col);
  const char* (*get_string)(int row, int col);
};
struct lmapi_sql_dataset* lmapi_sql_open(lmapi_t api, const char* sql);
void lmapi_sql_close(lmapi_t api, struct lmapi_sql_dataset* ds);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
#include <string>
#include <vector>
/** cpp-api */
namespace lmapi {

/** config read */
class config {
 public:
  friend class lmapi;

  ~config();

  bool is_open() const;

  /** value */
  int get_int(const std::string& name);
  int64_t get_int64(const std::string& name);
  float get_float(const std::string& name);
  double get_double(const std::string& name);
  std::string get_string(const std::string& name);

  /** array */
  std::vector<int> get_array_int(const std::string& name);
  std::vector<int64_t> get_array_int64(const std::string& name);
  std::vector<float> get_array_float(const std::string& name);
  std::vector<double> get_array_float64(const std::string& name);
  std::vector<std::string> get_array_string(const std::string& name);

 private:
  explicit config(const std::string& name);
  void* pdata;
};

class console {
 public:
  friend class lmapi;
  ~console();
  void debug(const char* format, ...);
  void info(const char* format, ...);
  void warning(const char* format, ...);
  void critical(const char* format, ...);
  void error(const char* format, ...);

 private:
  explicit console(int tp);
  void* pdata;
};

class sql_dataset {
 public:
  friend class lmapi;
  ~sql_dataset();

  const std::string& get_error(void) const;

  int rows(void) const;
  int cols(void) const;
  lmapi_sql_type type(int col) const;
  int get_int(int row, int col) const;
  int64_t get_int64(int row, int col) const;
  double get_float64(int row, int col) const;
  std::string get_string(int row, int col) const;

 private:
  explicit sql_dataset(const std::string& conn, const std::string& query);
  void* pdata;
};

class serial_dataset {
 public:
  friend class lmapi;
  ~serial_dataset();

 private:
  serial_dataset(int tp, const std::string& instrument, int start_date,
                 int end_date);
  void* pdata;
};

class factor_result {
 public:
  friend class lmapi;
  ~factor_result();

  const std::string& error() const;

  int store_factor(const lmapi_result_info& info);
  int store_result(const std::vector<lmapi_result_data>& ds);

 private:
  explicit factor_result(const std::string& conns,
                         const std::string& factor_name);
  void* pdata;
  std::string factor;
};

class lmapi {
 public:
  lmapi();
  ~lmapi();

  /** config read */
  config* config_open(const std::string& name);
  void config_close(config* cfg);

  /** console log */
  console* console_open(int tp);
  void console_close(console* con);

  /** data load */
  serial_dataset* serial_open(const std::string& instrument, int tp,
                              int start_date, int end_date);
  void serial_close(serial_dataset* ds);

  /** result store */
  factor_result* result_open(const std::string& factor_name);
  void result_close(factor_result* ds);

  /** sql query */
  sql_dataset* sql_open(const std::string& query);
  void sql_close(sql_dataset* ds);

 private:
  void* pdata;
};

}  // namespace lmapi

#endif

#endif  // LMAPI_LMAPI_H_

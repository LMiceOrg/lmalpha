/** Copyright 2018, 2019 He Hao<hehaoslj@sina.com> */
#ifndef INCLUDE_LMALPHA_H_
#define INCLUDE_LMALPHA_H_

#include "lmapi.h"
#include "lmstock.h"

#include <locale.h>
#include <stdint.h>
#include <stdio.h>

#include <chrono>
#include <memory>
#include <random>
#include <string>
#include <vector>

#if defined(_MSC_VER) /** MSC */
#define LMAPI_EXPORT __declspec(dllexport)
#else
#define LMAPI_EXPORT __attribute__((visibility("default")))
#endif

/** 高层API HLALPHA*/
#if defined(LM_ALPHA_HIGH_LEVEL_API)

/** 枚举数据类型 */
#define TICK LMAPI_TICK_TYPE
#define KMIN1 LMAPI_1MIN_TYPE
#define KMIN5 LMAPI_5MIN_TYPE
#define KMIN15 LMAPI_15MIN_TYPE
#define KMIN30 LMAPI_30MIN_TYPE
#define KMIN60 LMAPI_1HOUR_TYPE
#define KMIN120 LMAPI_2HOUR_TYPE

/** 初始化 */
#define LMAPI_INIT(x)                                     \
  setlocale(LC_ALL, "");                                  \
  std::shared_ptr<lmapi::lmapi> _s_api(new lmapi::lmapi); \
  std::shared_ptr<lmapi::config> _s_cfg;                  \
  std::shared_ptr<lmapi::console> _s_log;                 \
  _s_cfg.reset(_s_api->config_open((x)));                 \
  _s_log.reset(_s_api->console_open(LMAPI_LOG_DATETIME | LMAPI_LOG_THREAD));

/** 数据库查询 */
#define SQLQUERY(x) std::shared_ptr<lmapi::sql_dataset>(_s_api->sql_open((x)))

/** 日志功能 */
#define INFO(...) _s_log->info(__VA_ARGS__)
#define WARNING(...) _s_log->warning(__VA_ARGS__)
#define CRITICAL(...) _s_log->critical(__VA_ARGS__)
#define LOG(...) _s_log->debug(__VA_ARGS__)
#define DEBUG(...) LOG(__VA_ARGS__)
#define ERROR(...) _s_log->error(__VA_ARGS__)

#define ARR_INT(name) std::vector<int> name
#define ARR_DBL_LIST(name) std::vector<std::vector<double>> name

/** 时间序列数据查询 */
//#define ARR_KDATA(code_list, type, begin, end)          \
//  [_s_api]()std::shared_ptr<lmapi::serial_dataset>(               \
//      _s_api->serial_open(code_list, type, begin, end)) \
//      ->get_kdatas()

//#define ARR_TICK(code_list, begin, end)                 \
//  std::shared_ptr<lmapi::serial_dataset>(               \
//      _s_api->serial_open(code_list, TICK, begin, end)) \
//      ->get_ticks()

#define ARR_KDATA(code_list, type, begin, end)                                 \
  [_s_api](const std::vector<std::string> &instrument, int tp, int start_date, \
           int end_date) {                                                     \
    std::shared_ptr<lmapi::serial_dataset> p(                                  \
        _s_api->serial_open(instrument, tp, start_date, end_date));            \
    return p->get_kdatas();                                                    \
  }(code_list, type, begin, end);

#define ARR_TICK(code_list, begin, end)                                \
  [_s_api](const std::vector<std::string> &instrument, int start_date, \
           int end_date) {                                             \
    std::shared_ptr<lmapi::serial_dataset> p(                          \
        _s_api->serial_open(instrument, TICK, start_date, end_date));  \
    return p->get_ticks();                                             \
  }(code_list, begin, end)

/** 配置文件访问 */
#define CFG_STR(x) _s_cfg->get_string((x))
#define CFG_INT(x) _s_cfg->get_int((x))
#define CFG_DBL(x) _s_cfg->get_double((x))

#define CFG_ARR_INT(x) _s_cfg->get_array_int(x)
#define CFG_ARR_STR(x) _s_cfg->get_array_string(x)
#define CFG_ARR_DBL(x) _s_cfg->get_array_float64(x)

/** 因子计算结果存储 */
#define PREPARE_RESULT(name, stock_list) \
  std::vector<std::vector<lmapi_result_data>> name(stock_list.size());

//#define FACTOR_RESULT(name, stocks, results)                       \
//  std::shared_ptr<lmapi::factor_result>(_s_api->result_open(name)) \
//      ->store(stocks, results)

#define FACTOR_RESULT(name, stocks, results)                                 \
  [_s_api](const std::string &name,                                          \
           const std::vector<std::string> &stock_list,                       \
           const std::vector<std::vector<lmapi_result_data>> &result_list) { \
    std::shared_ptr<lmapi::factor_result> p(_s_api->result_open(name));      \
    p->store(stock_list, result_list);                                       \
  }(name, stocks, results)

/** 辅助函数库 */
extern "C" int lmlm(const double *y_val, const double *x_val, int y_size,
                    int x_size, double *rsqured);

typedef void *lmlib_lmsolver;
extern "C" lmlib_lmsolver lmlib_lmsolver_open(size_t n, size_t p);
extern "C" double lmlib_lmsolver_rsqured(lmlib_lmsolver lms,
                                         const double *y_val,
                                         const double *x_val);
extern "C" void lmlib_lmsolver_close(lmlib_lmsolver lms);
/* 线性回归 */
#define lm(a, b, c, d, e) lmlm(a, b, c, d, e)

#define LMOPEN(serial_size, param_size)                          \
  [](size_t n, size_t p) {                                       \
    class lmapi_lmsolver_helper {                                \
      lmlib_lmsolver solver;                                     \
                                                                 \
     public:                                                     \
      lmapi_lmsolver_helper(size_t n, size_t p) {                \
        solver = lmlib_lmsolver_open(n, p);                      \
      }                                                          \
      ~lmapi_lmsolver_helper() { lmlib_lmsolver_close(solver); } \
      double operator()(const double *y, const double *x) {      \
        return lmlib_lmsolver_rsqured(solver, y, x);             \
      }                                                          \
    };                                                           \
    std::unique_ptr<lmapi_lmsolver_helper> solver;               \
    solver.reset(new lmapi_lmsolver_helper(n, p));               \
    return solver;                                               \
  }(serial_size, param_size)

#define LMRSQ(s, y, x) s->operator()(y, x)
#define LMCLOSE(s) s.reset()
#define LMSOLVER(n, p) LMOPEN(n, p)

/* 时间函数 */
#define TIMENOW() std::chrono::system_clock::now()
#define DURATION(begin, end)                              \
  std::chrono::duration<double>(end.time_since_epoch() -  \
                                begin.time_since_epoch()) \
      .count()
#endif

#endif /** INCLUDE_LMALPHA_H_ */

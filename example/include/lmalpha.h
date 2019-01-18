/** Copyright 2018, 2019 He Hao<hehaoslj@sina.com> */
#ifndef INCLUDE_LMALPHA_H_
#define INCLUDE_LMALPHA_H_

#include <stdio.h>
#include <stdint.h>
#include <locale.h>

#include <chrono>
#include <memory>
#include <random>
#include <string>
#include <vector>

#include "lmapi.h"
#include "lmstock.h"

#if defined(_MSC_VER) /** MSC */
#define LMAPI_EXPORT __declspec(dllexport)
#else
#define LMAPI_EXPORT __attribute__((visibility("default")))
#endif




/** 高层API HLALPHA*/
#if defined(LM_ALPHA_HIGH_LEVEL_API)

/** 枚举数据类型 */
#define TICK LMAPI_TICK_TPYE
#define KMIN1 LMAPI_1MIN_TYPE
#define KMIN5 LMAPI_5MIN_TYPE
#define KMIN15 LMAPI_15MIN_TYPE
#define KMIN30 LMAPI_30MIN_TYPE
#define KMIN60 LMAPI_1HOUR_TYPE
#define KMIN120 LMAPI_2HOUR_TYPE

/** 初始化 */
#define LMAPI_INIT(x) setlocale(LC_ALL, ""); \
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
#define ARR_KDATA(code_list, type, begin, end)          \
  std::shared_ptr<lmapi::serial_dataset>(               \
      _s_api->serial_open(code_list, type, begin, end)) \
      ->get_kdatas()

#define ARR_TICK(code_list, begin, end)                 \
  std::shared_ptr<lmapi::serial_dataset>(               \
      _s_api->serial_open(code_list, TICK, begin, end)) \
      ->get_ticks()

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

#define FACTOR_RESULT(name, stocks, results)                       \
  std::shared_ptr<lmapi::factor_result>(_s_api->result_open(name)) \
      ->store(stocks, results)

/** 辅助函数库 */
extern "C" int lmlm(const double *y_val, const double *x_val, int y_size,
                    int x_size, double *rsqured);
/* 线性回归 */
#define lm(a, b, c, d, e) lmlm(a, b, c, d, e)

/* 时间函数 */
#define TIMENOW() std::chrono::system_clock::now()
#define DURATION(begin, end)                              \
  std::chrono::duration<double>(end.time_since_epoch() -  \
                                begin.time_since_epoch()) \
      .count()
#endif

#endif /** INCLUDE_LMALPHA_H_ */

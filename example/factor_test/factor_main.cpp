#include <stdio.h>

#include "lmapi.h"
#include "lmstock.h"

#if defined(_MSC_VER) /** MSC */
#define LMAPI_EXPORT __declspec(dllexport)
#else
#define LMAPI_EXPORT __attribute__((visibility("default")))
#endif

#define KMIN60 lmkdata::Min_60

#define LMAPI_INIT(x)                                     \
  std::shared_ptr<lmapi::lmapi> _s_api(new lmapi::lmapi); \
  std::shared_ptr<lmapi::config> _s_cfg;                  \
  std::shared_ptr<lmapi::console> _s_log;                 \
  _s_cfg.reset(_s_api->config_open((x)));                 \
  _s_log.reset(_s_api->console_open(LMAPI_LOG_DATETIME | LMAPI_LOG_THREAD));

#define SQLQUERY(x) std::shared_ptr<lmapi::sql_dataset>(_s_api->sql_open((x)))

#define INFO(...) _s_log->info(__VA_ARGS__)
#define CRITICAL(...) _s_log->critical(__VA_ARGS__)
#define LOG(...) _s_log->debug(__VA_ARGS__)
#define DEBUG(...) LOG(__VA_ARGS__)
#define ERROR(...) _s_log->error(__VA_ARGS__)

#define ARR_INT(name) std::vector<int> name
#define ARR_DBL_LIST(name) std::vector<std::vector<double>> name
#define KDATA(name) std::vector<lmkdata> name
#define ARR_KDATA(name) std::vector<std::vector<lmkdata>> name
#define KDATA_LOAD(code, begin, end, type)
#define ARR_KDATA_LOAD(code_list, begin, end, type) \
  std::vector<std::vector<lmkdata>>()

#define CFG_STR(x) _s_cfg->get_string((x))
#define CFG_INT(x) _s_cfg->get_int((x))
#define CFG_DBL(x) _s_cfg->get_double((x))
#define CFG_ARR_STR(x, arr)                            \
  do {                                                 \
    int sz = _s_cfg->get_array_size((x));              \
    for (int i = 0; i < sz; ++i) {                     \
      arr.push_back(_s_cfg->get_array_string((x), i)); \
    }                                                  \
  } while (0)

#define CFG_ARR_INT(x) _s_cfg->get_array_int(x)

#define CFG_ARR_DBL(x, arr)                            \
  do {                                                 \
    int sz = _s_cfg->get_array_size((x));              \
    for (int i = 0; i < sz; ++i) {                     \
      arr.push_back(_s_cfg->get_array_double((x), i)); \
    }                                                  \
  } while (0)

#define CLOSEPRICE kdata.closePrice
#define PRECLOSEPRICE kdata.preClosePrice

#define ARR_KDATA_CALC(rt, arr, pred)             \
  do {                                            \
    for (size_t i = 0; i < arr.size(); ++i) {     \
      const auto& klist = arr[i];                 \
      std::vector<double> vals;                   \
      for (size_t j = 0; j < klist.size(); ++j) { \
        const auto& kdata = klist[j];             \
        auto value = pred;                        \
        vals.push_back(value);                    \
      }                                           \
      rt.push_back(vals);                         \
    }                                             \
  } while (0)

extern "C" {
LMAPI_EXPORT void factor_run(const char* cfg_name) {
  LMAPI_INIT(cfg_name);

  CRITICAL("factor run\n");

  //  1. 读取配置文件
  DEBUG("1. 读取配置文件\n");
  auto name = CFG_STR("BasicInfo.FactorName");
  auto begin = CFG_INT("BackTest.nBeginDate");
  auto end = CFG_INT("BackTest.nEndDate");

  auto stock_list = CFG_ARR_INT("BackTest.StockList");
  auto index_list = CFG_ARR_INT("BackTest.IndexList");

  INFO("config:factor name: %s\n", name.c_str());
  INFO("config:BackTest from %08d to %08d\n", begin, end);

  //  2. 数据获取
  DEBUG("2. 数据获取\n");
  INFO("读取股票 60分钟k线");
  for (size_t i = 0; i < stock_list.size(); ++i) printf(" %06d", stock_list[i]);
  printf("\n");

  ARR_KDATA(stock_kmin60);
  stock_kmin60 = ARR_KDATA_LOAD(stock_list, begin, end, KMIN60);

  INFO("读取指数 60分钟k线");
  for (size_t i = 0; i < index_list.size(); ++i) printf(" %06d", index_list[i]);
  printf("\n");
  ARR_KDATA(index_kmin60);
  index_kmin60 = ARR_KDATA_LOAD(index_list, begin, end, KMIN60);

  INFO("计算百分比变动\n");
  ARR_DBL_LIST(pct_change);
  ARR_KDATA_CALC(pct_change, stock_kmin60,
                 (CLOSEPRICE - PRECLOSEPRICE / PRECLOSEPRICE));

  INFO("获取每日总股本数据\n");
  auto query = SQLQUERY(
      "select top 15 TradingDay"
      ",SecuCode"
      ",Ashares"
      " from General.dbo.DailyQuote"
      " where  Flg =1 and "
      " (TradingDay between '2017-01-01' and '2018-12-31');");
  INFO("股本数据行数 %d\n", query->rows());
  if (!query->get_error().empty()) {
    ERROR("%s\n", query->get_error().c_str());
  }
  for (int i = 0; i < query->rows(); ++i) {
    for (int j = 0; j < query->cols(); ++j) {
      printf("  %s,", query->get_string(i, j).c_str());
    }
    printf("\n");
  }
  INFO("计算总市值的60分钟线\n");
  ARR_DBL_LIST(MktCap_Min_60);

  DEBUG("3. 数据清洗\n");
  DEBUG("4. 因子计算\n");
  DEBUG("5. 结果存储\n");
}
}

#include <stdio.h>

#include "lmapi.h"
#include "lmstock.h"
extern "C" int lm(double* y_val, double* x_val, int y_size, int x_size);

#if defined(_MSC_VER) /** MSC */
#define LMAPI_EXPORT __declspec(dllexport)
#else
#define LMAPI_EXPORT __attribute__((visibility("default")))
#endif

#define KMIN1 LMAPI_1MIN_TYPE
#define KMIN60 LMAPI_1HOUR_TYPE

#define LMAPI_INIT(x)                                     \
  std::shared_ptr<lmapi::lmapi> _s_api(new lmapi::lmapi); \
  std::shared_ptr<lmapi::config> _s_cfg;                  \
  std::shared_ptr<lmapi::console> _s_log;                 \
  _s_cfg.reset(_s_api->config_open((x)));                 \
  _s_log.reset(_s_api->console_open(LMAPI_LOG_DATETIME | LMAPI_LOG_THREAD));

#define SQLQUERY(x) std::shared_ptr<lmapi::sql_dataset>(_s_api->sql_open((x)))

#define INFO(...) _s_log->info(__VA_ARGS__)
#define WARNING(...) _s_log->warning(__VA_ARGS__)
#define CRITICAL(...) _s_log->critical(__VA_ARGS__)
#define LOG(...) _s_log->debug(__VA_ARGS__)
#define DEBUG(...) LOG(__VA_ARGS__)
#define ERROR(...) _s_log->error(__VA_ARGS__)

#define ARR_INT(name) std::vector<int> name
#define ARR_DBL_LIST(name) std::vector<std::vector<double>> name
#define KDATA(name) std::vector<lmkdata> name

#define ARR_KDATA(code_list, type, begin, end)          \
  std::shared_ptr<lmapi::serial_dataset>(               \
      _s_api->serial_open(code_list, type, begin, end)) \
      ->get_kdatas()

#define KDATA_LOAD(code, begin, end, type)
#define ARR_KDATA_LOAD(code_list, begin, end, type) \
  std::vector<std::vector<lmkdata>>()

#define CFG_STR(x) _s_cfg->get_string((x))
#define CFG_INT(x) _s_cfg->get_int((x))
#define CFG_DBL(x) _s_cfg->get_double((x))

#define CFG_ARR_INT(x) _s_cfg->get_array_int(x)
#define CFG_ARR_STR(x) _s_cfg->get_array_string(x)
#define CFG_ARR_DBL(x) _s_cfg->get_array_float64(x)

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

struct Data {
  std::string name;
  int gender;  // 1 male, 0 female
  int age;     // years
  int height;  // inches
  int weight;  // pounds
};

Data data[] = {{"Joe", 1, 25, 72, 178},
               {"Jill", 0, 32, 68, 122},
               {"Jack", 1, 27, 69, 167},
               {"John", 1, 45, 67, 210},
               {"Jane", 0, 38, 62, 108}};

#include <chrono>
#include <random>

extern "C" {
LMAPI_EXPORT void factor_run(const char* cfg_name) {
  LMAPI_INIT(cfg_name);

  CRITICAL("factor run\n");

  DEBUG("test lm\n");
  std::vector<double> y;

  for (int i = 0; i < 5; ++i) {
    y.push_back(data[i].weight);
  }

  std::vector<double> x;
  for (int i = 0; i < 5; ++i) {
    x.push_back(data[i].gender);
    x.push_back(data[i].age);
    x.push_back(data[i].height);
  }
  auto tstart = std::chrono::system_clock::now();
  lm(&y[0], &x[0], 5, 15);
  auto tend = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed_seconds = tend - tstart;
  CRITICAL("LM time %lf\n", elapsed_seconds.count());

  //  1. 读取配置文件
  DEBUG("1. 读取配置文件\n");
  auto name = CFG_STR("BasicInfo.FactorName");
  auto begin = CFG_INT("BackTest.nBeginDate");
  auto end = CFG_INT("BackTest.nEndDate");

  auto stock_list = CFG_ARR_STR("BackTest.StockList");
  auto index_list = CFG_ARR_STR("BackTest.IndexList");

  INFO("config:factor name: %s\n", name.c_str());
  INFO("config:BackTest from %08d to %08d\n", begin, end);

  //  2. 数据获取
  DEBUG("2. 数据获取\n");
  INFO("读取股票 60分钟k线");
  for (size_t i = 0; i < stock_list.size(); ++i)
    printf(" %s", stock_list[i].c_str());
  printf("\n");

  // 获取 k线数据
  auto stock_kmin1 = ARR_KDATA(stock_list, KMIN1, begin, end);
  // 输出调试信息
  WARNING("stock size %lu\n", stock_kmin1.size());
  for (size_t i = 0; i < stock_kmin1.size(); ++i) {
    printf("stock: %s\tsize: %lu\n", stock_list[i].c_str(),
           stock_kmin1[i].size());
  }

  INFO("读取指数 60分钟k线");
  for (size_t i = 0; i < index_list.size(); ++i)
    printf(" %s", index_list[i].c_str());
  printf("\n");

  auto index_kmin60 = ARR_KDATA(index_list, KMIN60, begin, end);
  WARNING("index size:%lu\n", index_kmin60.size());

  INFO("计算百分比变动\n");
  //  ARR_DBL_LIST(pct_change);
  //  ARR_KDATA_CALC(pct_change, stock_kmin60,
  //                 (CLOSEPRICE - PRECLOSEPRICE / PRECLOSEPRICE));

  INFO("获取每日总股本数据\n");
  //  auto query = SQLQUERY(
  //      "select top 15 TradingDay"
  //      ",SecuCode"
  //      ",Ashares"
  //      " from General.dbo.DailyQuote"
  //      " where  Flg =1 and "
  //      " (TradingDay between '2017-01-01' and '2018-12-31');");
  //  INFO("股本数据行数 %d\n", query->rows());
  //  if (!query->get_error().empty()) {
  //    ERROR("%s\n", query->get_error().c_str());
  //  }
  //  for (int i = 0; i < query->rows(); ++i) {
  //    for (int j = 0; j < query->cols(); ++j) {
  //      printf("  %s,", query->get_string(i, j).c_str());
  //    }
  //    printf("\n");
  //  }
  INFO("计算总市值的60分钟线\n");
  ARR_DBL_LIST(MktCap_Min_60);

  DEBUG("3. 数据清洗\n");
  DEBUG("4. 因子计算\n");
  std::minstd_rand r;

  tstart = std::chrono::system_clock::now();
  for (size_t i = 0; i < stock_list.size(); ++i) {
    const auto& stock = stock_list[i];
    const auto& stock_kdata = stock_kmin1[i];
    if (stock_kdata.size() < 20) continue;
    INFO("计算 %s PCT_CHANGE\n", stock.c_str());
    for (std::vector<lmkdata>::const_iterator bar = stock_kdata.begin() + 20;
         bar != stock_kdata.end(); ++bar) {
      auto bar20 = bar - 20;

      std::vector<double> stock_pct(20);
      int pos = 0;
      for (auto it = bar20; it != bar; ++it, ++pos) {
        stock_pct.push_back((it->closePrice - it->preClosePrice) /
                            it->preClosePrice);
      }

      std::vector<double> index_pct;

      for (size_t idx = 0; idx < 20; ++idx) {
        index_pct.push_back(r() / 1e10);
      }

      lm(&index_pct[0], &stock_pct[0], 20, 20);
    }
  }
  tend = std::chrono::system_clock::now();
  elapsed_seconds = tend - tstart;
  CRITICAL("因子计算时间 %lf\n", elapsed_seconds.count());
  DEBUG("5. 结果存储\n");
}
}

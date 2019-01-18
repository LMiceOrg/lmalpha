/** Copyright 2018, 2019 He Hao<hehaoslj@sina.com> */
#define LM_ALPHA_HIGH_LEVEL_API
#include "../include/lmalpha.h"

#include <stdio.h>

/** Qt for Mingw32 : cmake parameter
 * cmake -DCMAKE_CXX_COMPILER=e:/msys32/mingw32/bin/g++.exe -DCMAKE_C_COMPILER=e:/msys32/mingw32/bin/gcc.exe -G "MSYS Makefiles"
 */

extern "C" {
LMAPI_EXPORT void factor_run(const char *cfg_name) {
  LMAPI_INIT(cfg_name);

  CRITICAL("factor run\n");
  

  //  DEBUG("test lm\n");
  //  std::vector<double> y;

  //  for (int i = 0; i < 5; ++i) {
  //    y.push_back(data[i].weight);
  //  }

  //  std::vector<double> x;
  //  for (int i = 0; i < 5; ++i) {
  //    x.push_back(data[i].gender);
  //    x.push_back(data[i].age);
  //    x.push_back(data[i].height);
  //  }
  //  auto tstart = TIMENOW();
  //  double rsq;
  //  lm(&y[0], &x[0], 5, 15, &rsq);
  //  auto tend = TIMENOW();
  //  auto elapsed_seconds = DURATION(tstart, tend);
  //  CRITICAL("LM time %lf\n", elapsed_seconds);

  //  1. 读取配置文件
  DEBUG("%ls", L"1. 读取配置文件\n");
  auto name = CFG_STR("BasicInfo.FactorName");
  auto begin = CFG_INT("BackTest.nBeginDate");
  auto end = CFG_INT("BackTest.nEndDate");

  auto stock_list = CFG_ARR_STR("BackTest.StockList");
  auto index_list = CFG_ARR_STR("BackTest.IndexList");

  INFO("config:factor name: %s\n", name.c_str());
  INFO("config:BackTest from %08d to %08d\n", begin, end);

  //  2. 数据获取
  DEBUG("%ls", L"2. 数据获取\n");
  INFO("%ls", L"读取股票 60分钟k线\n");
  for (size_t i = 0; i < stock_list.size(); ++i)
    printf(" %s", stock_list[i].c_str());
  printf("\n");

  // 获取 k线数据
  
	  auto p=std::shared_ptr<lmapi::serial_dataset>(
	  _s_api->serial_open(stock_list, KMIN1, begin, end));
	  auto stock_kmin1 = p->get_kdatas();
  //auto stock_kmin1 = ARR_KDATA(stock_list, KMIN1, begin, end);
  // 输出调试信息
  WARNING("stock size %lu\n", stock_kmin1.size());
  for (size_t i = 0; i < stock_kmin1.size(); ++i) {
    printf("stock: %s\tsize: %lu\n", stock_list[i].c_str(),
           stock_kmin1[i].size());
  }

  INFO("%ls", L"读取指数 60分钟k线\n");
  for (size_t i = 0; i < index_list.size(); ++i)
    printf(" %s", index_list[i].c_str());
  printf("\n"); 

  auto index_kmin60 = ARR_KDATA(index_list, KMIN60, begin, end);
  WARNING("index size:%lu\n", index_kmin60.size());

  INFO("%ls", L"获取每日总股本数据\n");
  auto query = SQLQUERY(
      "select top 15 TradingDay"
      ",SecuCode"
      ",Ashares"
      " from General.dbo.DailyQuote"
      " where  Flg =1 and "
      " (TradingDay between '2017-01-01' and '2018-12-31');");
  INFO("%ls %d\n", L"股本数据行数", query->rows());
  if (!query->get_error().empty()) {
    ERROR("%s\n", query->get_error().c_str());
  }
  for (int i = 0; i < query->rows(); ++i) {
    for (int j = 0; j < query->cols(); ++j) {
      printf("  %s,", query->get_string(i, j).c_str());
    }
    printf("\n");
  }
  INFO("%ls", L"计算总市值的60分钟线\n");
  ARR_DBL_LIST(MktCap_Min_60);

  DEBUG("%ls", L"3. 数据清洗\n");
  DEBUG("%ls", L"4. 因子计算\n");
  std::minstd_rand r;
  PREPARE_RESULT(result_list, stock_list);

  auto tstart = TIMENOW();
  for (size_t i = 0; i < stock_list.size(); ++i) {
    const auto &stock = stock_list[i];
    const auto &stock_kdata = stock_kmin1[i];
    auto &result_dataset = result_list[i];
    if (stock_kdata.size() < 20) continue;
    INFO("%ls %s\n", L"计算 百分比变动", stock.c_str());
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
      double rsq = 0;
      lm(&index_pct[0], &stock_pct[0], 20, 20, &rsq);

      lmapi_result_data result;
      result.date = bar->nDate;
      result.time = bar->nTimeBegin;
      result.value = rsq;
      result_dataset.push_back(result);
    }
  }
  auto tend = TIMENOW();
  auto elapsed_seconds = DURATION(tstart, tend);
  CRITICAL("%ls  %lf\n", L"因子计算时间", elapsed_seconds);
  DEBUG("%ls", L"5. 结果存储\n");

  lmapi::factor_result* p2 = _s_api->result_open(name);

  p2->store(stock_list, result_list);

  //[&](const std::string& name, const std::vector<std::string>& stock_list, const std::vector<std::vector<lmapi_result_data> >& result_list) {
	 // std::shared_ptr<lmapi::factor_result> p(_s_api->result_open(name));

		//  p->store(stock_list, result_list);
  //}(name, stock_list, result_list);
  //
  //FACTOR_RESULT(name, stock_list, result_list);
  INFO("%ls", L"因子计算完成\n");
}
}

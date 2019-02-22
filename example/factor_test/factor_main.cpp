/** Copyright 2018, 2019 He Hao<hehaoslj@sina.com> */
#define LM_ALPHA_HIGH_LEVEL_API
#include "../include/lmalpha.h"

#include <array>
#include <iostream>
#include <thread>
/** Qt for Mingw32 : cmake parameter
 * cmake -DCMAKE_CXX_COMPILER=e:/msys32/mingw32/bin/g++.exe
 * -DCMAKE_C_COMPILER=e:/msys32/mingw32/bin/gcc.exe -G "MSYS Makefiles"
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
  //  auto stock_kmin1 = [_s_api](const std::vector<std::string> &instrument,
  //                              int tp, int start_date, int end_date) {
  //    std::shared_ptr<lmapi::serial_dataset> p(
  //        _s_api->serial_open(instrument, tp, start_date, end_date));
  //    return p->get_kdatas();
  //  }(stock_list, KMIN1, begin, end);

  auto stock_kmin1 = ARR_KDATA(stock_list, KMIN1, begin, end);

  //  auto p = std::shared_ptr<lmapi::serial_dataset>(
  //      _s_api->serial_open(stock_list, KMIN1, begin, end));
  //  auto stock_kmin1 = p->get_kdatas();
  // auto stock_kmin1 = ARR_KDATA(stock_list, KMIN1, begin, end);
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
  auto read_begin = TIMENOW();
  auto index_kmin60 = ARR_KDATA(index_list, KMIN60, begin, end);
  auto read_end = TIMENOW();
  auto read_time = DURATION(read_begin, read_end);

  WARNING("index size:%lu  time %lf\n", index_kmin60.size(), read_time);

  auto tick_list = CFG_ARR_STR("BackTest.tickList");
  auto ticks = ARR_TICK(tick_list, begin, end);
  WARNING("tick list size %lu %d %d\n", ticks.size(), begin, end);

  for (size_t pos = 0; pos < ticks.size(); ++pos) {
    auto const &stock = stock_list[pos];
    auto const &stock_ticks = ticks[pos];
    std::cout << "bytes:" << stock_ticks.bytes() << std::endl;
    for (auto const &day_tick : stock_ticks) {
      int day = day_tick.first;
      auto const &day_serial = day_tick.second;
      for (auto const &tick_data : day_serial) {
        // 遍历 lmtickdata
        std::cout << "stock " << stock /* stock */
                  << " day " << day    /* day */
                  << " " << tick_data  /* tick */
                  << std::endl;
      }
    }
  }

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
  //  INFO("%ls", L"计算总市值的60分钟线\n");
  //  ARR_DBL_LIST(MktCap_Min_60);

  DEBUG("%ls", L"3. 数据清洗\n");
  DEBUG("%ls", L"4. 因子计算\n");

  /** performance test only 
    stocks
    1 year (250 days)
    kmin1 4 hours (60 minutes)
  */
  {
      std::minstd_rand r;
      r.seed(1234);

      char buffer[32];

      stock_kmin1.clear();
      stock_list.clear();
      time_t now;
      time(&now);
      tm date;

      size_t timebegin_minute = 9 * 60+30;
      size_t stock_count = 200;
      

      // stock_count 股票名称 k线数据
      for (size_t i = 0; i < stock_count; ++i) {
          std::string name = itoa(600000 + i, buffer, 10);
          stock_list.push_back(name);

          std::vector<lmkdata> kdatas;
          for (size_t day = 0; day < 250; ++day) {
              for (size_t minute = 0; minute < 60 * 4; ++minute) {
                  time_t cur =now + 86400 * day;
                  _gmtime64_s(&date, &cur);
                  size_t timeminute = timebegin_minute + minute;
                  lmkdata dt;
                  dt.volume = 120000;
                  dt.preClosePrice = (12.75*r())/ 2147483647;
                  dt.openPrice = (12.35*r()) / 2147483647;
                  dt.closePrice = (12.75*r()) / 2147483647;
                  dt.dealCount = 10000;
                  dt.highPrice = (13.35*r()) / 2147483647;
                  dt.lowPrice = (12.25*r()) / 2147483647;
                  dt.money = 126500;
                  dt.nCode = 600000 + i;
                  dt.nDate = date.tm_year*10000 + date.tm_mon*100+date.tm_mday;
                  dt.nTimeBegin = (timeminute / 60) * 10000 + (timeminute % 60) * 100;
                  dt.nType = KMIN1;
                  kdatas.push_back(dt);
                  
              }
          }
          stock_kmin1.push_back(kdatas);

      }
  }

  PREPARE_RESULT(result_list, stock_list);

  printf("stock list %ld %ld %ld\n", stock_list.size(), result_list.size(), stock_kmin1.size());

  /** 定义求解函数 */
  auto factor_solver = [&](size_t count,     /** 回归序列长度 */
                           size_t task_rank, /** 当前任务ID */
                           size_t task_size) /** 任务总数 */
      -> void {
    std::minstd_rand r;                               /** 随机数生成器*/
    size_t p_size = 2;                                /** 回归变量个数 */
    std::vector<double> stock_pct(count);             /** 数组 stock_pct */
    std::vector<double> index_pct(count * p_size, 1); /** 数组 index_pct */

    auto solver = LMSOLVER(count, p_size); /** 回归求解器 */
    const double *y_val = &stock_pct[0];   /** 回归 y值 */
    const double *x_val = &index_pct[0];   /** 回归 x值 */

    /** 当前任务 开始计算位置 */
    size_t stock_begin = task_rank * stock_list.size() / task_size;
    /** 当前任务 结束计算位置 */
    size_t stock_end = (task_rank + 1) * stock_list.size() / task_size;
    for (size_t i = stock_begin; i < stock_end; ++i) {
      auto const &stock = stock_list[i];
      auto const &stock_kdata = stock_kmin1[i];
      auto &result_dataset = result_list[i];
      // printf("stock kdata size %ld\n", stock_kdata.size());
      if (stock_kdata.size() < count) continue;
      // INFO(" %ls code: %s\n", L"计算 百分比变动", stock.c_str());
      // std::remove_reference<decltype(stock_kdata)>::type::const_iterator bar;
      // std::decay<decltype(stock_kdata)>::type::const_iterator bar;
      // for (size_t j = 0; j < 1500; ++j)
      size_t result_idx = 0;
      for(size_t bar_pos = count; bar_pos <= stock_kdata.size(); ++bar_pos, ++result_idx)
       {                                /** 步长 1 */
        auto bar_step_begin = stock_kdata.begin() + bar_pos - count;
        auto bar_step_end = stock_kdata.begin() + bar_pos;

        // std::vector<double> stock_pct(count);
        int pos = 0;
        for (auto it = bar_step_begin; it != bar_step_end; ++it, ++pos) {
          //  替换 stock_pct.push_back
          stock_pct[pos] =
              ((it->closePrice - it->preClosePrice) / it->preClosePrice);
        }
        // std::vector<double> index_pct(count);
        for (size_t idx = 0; idx < count * p_size; idx += p_size) {
          // 第0列 截距参数 总是为1
          // index_pct[idx] = 1;

          // 第1列 设置x1参数
          // 测试使用 设置随机数作为index
          index_pct[idx + 1] = (r() / 1e10);
        }

        // double rsq1 = 0;
        // lm(&stock_pct[0], &index2_pct[0], count, count, &rsq1);
        double rsq = LMRSQ(solver, y_val, x_val);
        // printf("\t lm=%lf\t LM=%lf\n", rsq1, rsq);

        // 替换result_dataset.push_back
        auto &result = result_dataset[result_idx];
        result.date = bar_step_begin->nDate;
        result.time = bar_step_begin->nTimeBegin;
        result.value = isnormal(rsq) ? rsq : (rsq == 0 ? 0 : 1);
        
        // result_dataset.push_back(result);

        // printf("\tresult dt %d%d %lf\n",result.date, result.time, result.value);

      }  // for-end: bar_pos
    }  // for-end:i
  };

  auto clear_result = [&result_list, &stock_kmin1](size_t count) -> void {
    for (size_t i = 0; i < result_list.size(); ++i) {
        if (stock_kmin1[i].size() < count) {
            result_list[i].resize(0);
        } 
        else
        {
            result_list[i].resize(stock_kmin1[i].size() - count + 1);
        }
      
    }
  };

  // 第1次 调用计算 步长20
  auto tstart = TIMENOW();
  size_t step_count = 20;
  const size_t thread_size = std::thread::hardware_concurrency();
  clear_result(step_count);
  printf("thread size %lu\n", thread_size);
  std::vector<std::unique_ptr<std::thread> > tasks(thread_size);
  for (size_t i = 0; i < thread_size; ++i) {
    tasks[i].reset(new std::thread(factor_solver, step_count, i, thread_size));
  }
  for (size_t i = 0; i < thread_size; ++i) {
    tasks[i]->join();
  }
  auto tend = TIMENOW();
  auto elapsed_seconds = DURATION(tstart, tend);
  CRITICAL("\t%ls[%lu]  %lf\n", L"因子计算时间", step_count, elapsed_seconds);

  // 第2次 调用计算 步长50
  //tstart = TIMENOW();
  //step_count = 50;
  //clear_result(step_count);
  //for (size_t i = 0; i < thread_size; ++i) {
  //  tasks[i].reset(new std::thread(factor_solver, step_count, i, thread_size));
  //}
  //for (size_t i = 0; i < thread_size; ++i) {
  //  tasks[i]->join();
  //}
  //tend = TIMENOW();
  //elapsed_seconds = DURATION(tstart, tend);
  //CRITICAL("\t%ls[%lu]  %lf\n", L"因子计算时间", step_count, elapsed_seconds);

  // 第3次 调用计算 步长50
  //tstart = TIMENOW();
  //step_count = 100;
  //clear_result(step_count);
  //for (size_t i = 0; i < thread_size; ++i) {
  //  tasks[i].reset(new std::thread(factor_solver, step_count, i, thread_size));
  //}
  //for (size_t i = 0; i < thread_size; ++i) {
  //  tasks[i]->join();
  //}
  //tend = TIMENOW();
  //elapsed_seconds = DURATION(tstart, tend);
  //CRITICAL("\t%ls[%lu]  %lf\n", L"因子计算时间", step_count, elapsed_seconds);

  DEBUG("%ls", L"5. 结果存储\n");

  FACTOR_RESULT(name, stock_list, result_list);

  INFO("%ls", L"因子计算完成\n");
}

}  // extern C

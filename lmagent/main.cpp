#include <stdio.h>
#include <iostream>
#include "../lmapi/lmapi.h"

#include <dlfcn.h>

extern int lm_run_file(const char* name);

int main(int argc, char** argv) {
  if (argc != 2) {
    printf(
        "Usage:\n"
        "  [app] [config file(json format)]\n"
        "\teg: lmagent test.json\n");
    return -1;
  }
  /*
    lmapi::lmapi* api = new lmapi::lmapi();
    lmapi::config* cfg = api->config_open(argv[1]);
    lmapi::console* console_date = api->console_open(LMAPI_LOG_DATETIME);
    lmapi::console* console_all = api->console_open(LMAPI_LOG_ALL);
    // printf("cfg %p\n", cfg);
    printf("factor name %s\n", cfg->get_string("factor.name").c_str());
    std::cout << cfg->get_array_size("testing.instruments") << " inst[0] "
              << cfg->get_array_string("testing.instruments", 0) << " inst[1] "
              << cfg->get_array_string("testing.instruments", 1) << std::endl;

    api->config_close(cfg);

    console_date->error("date:buxihuan \n");
    console_date->info("date:shenme shiqing\n");
    console_date->warning("date:shenme shiqing\n");
    console_date->debug("date:shenme shiqing\n");
    console_date->critical("date:shenme shiqing\n");

    console_all->error("all:buxihuan \n");
    console_all->info("all:shenme shiqing\n");
    console_all->warning("all:shenme shiqing\n");
    console_all->debug("all:shenme shiqing\n");
    console_all->critical("all:shenme shiqing\n");

    console_date->critical(
        "sql: select top 5 id, IndustryName, UpdateTime from C_EX_Industry\n");
    lmapi::sql_dataset* ds = api->sql_open(
        "select top 5 id, IndustryName, UpdateTime "
        "from C_EX_Industry");
    if (!ds->get_error().empty())
      console_date->warning("sql: %s\n", ds->get_error().c_str());
    console_date->info("sql: rows:%d cols:%d\n", ds->rows(), ds->cols());
    for (int i = 0; i < ds->rows(); ++i) {
      for (int j = 0; j < ds->cols(); ++j) {
        if (j == 0)
          console_date->info("data[%d, %d]:%s\t", i, j,
                             ds->get_string(i, j).c_str());
        else
          printf("data[%d, %d]:%s\t", i, j, ds->get_string(i, j).c_str());
      }
      printf("\n");
    }
    api->sql_close(ds);

    lmapi::factor_result* rs = api->result_open("factor1");
    lmapi_result_info info;
    memset(&info, 0, sizeof(info));
    sprintf(info.factor_name, "factor1");
    sprintf(info.factor_author, "ly123");
    sprintf(info.factor_date, "20190112");
    rs->store_factor(info);
    console_date->info(
        "result: store_factor \nfactor:\t%s\nauthor:\t%s\ndate:\t%s\n",
        info.factor_name, info.factor_author, info.factor_date);

    console_date->info("result: store_result 10\n");
    std::vector<lmapi_result_data> results;
    for (size_t i = 0; i < 10; ++i) {
      lmapi_result_data ds;
      ds.date = 20190111;
      ds.time = 130320 + i;
      ds.value = i * 1.23563;

      results.push_back(ds);
      console_date->info("result[%lu]: value %.15lf\n", i, info.factor_name,
                         ds.value);
    }
    rs->store_result(results);

    api->console_close(console_date);
    api->console_close(console_all);

    delete api;
  */
  void* hdll = dlopen("libfactor_test.dylib", RTLD_LAZY);

  typedef void (*f_run)(const char* cfg);
  f_run func = (f_run)dlsym(hdll, "factor_run");
  pthread_setname_np("LMAPI-AGENT");
  func(argv[1]);
  return 0;
}

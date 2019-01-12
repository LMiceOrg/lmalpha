#include <stdio.h>

#include "lmapi.h"

#if defined(_MSC_VER) /** MSC */
#define LMAPI_EXPORT __declspec(dllexport)
#else
#define LMAPI_EXPORT __attribute__((visibility("default")))
#endif

#define LMAPI_INIT(x)                                     \
  std::shared_ptr<lmapi::lmapi> _s_api(new lmapi::lmapi); \
  std::shared_ptr<lmapi::config> _s_cfg;                  \
  std::shared_ptr<lmapi::console> _s_log;                 \
  _s_cfg.reset(_s_api->config_open((x)));                 \
  _s_log.reset(_s_api->console_open(LMAPI_LOG_DATETIME | LMAPI_LOG_THREAD));

#define INFO(x, ...) _s_log->info(x, __VA_ARGS__)
#define CRITICAL(...) _s_log->critical(__VA_ARGS__)
#define LOG(...) _s_log->debug(__VA_ARGS__)

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

#define CFG_ARR_INT(x, arr)                         \
  do {                                              \
    int sz = _s_cfg->get_array_size((x));           \
    for (int i = 0; i < sz; ++i) {                  \
      arr.push_back(_s_cfg->get_array_int((x), i)); \
    }                                               \
  } while (0)

#define CFG_ARR_DBL(x, arr)                            \
  do {                                                 \
    int sz = _s_cfg->get_array_size((x));              \
    for (int i = 0; i < sz; ++i) {                     \
      arr.push_back(_s_cfg->get_array_double((x), i)); \
    }                                                  \
  } while (0)

extern "C" {
LMAPI_EXPORT void factor_run(const char* cfg_name) {
  LMAPI_INIT(cfg_name);

  LOG("factor run\n");

  //  1. 读取配置文件
  auto name = CFG_STR("factor.name");
  auto length = CFG_INT("testing.data_length");

  INFO("config:factor name: %s\n", name.c_str());
  INFO("config:data size: %d\n", length);

  //  2. 数据获取
}
}

/** Copyright 2018, 2019 He Hao<hehaoslj@sina.com> */

#include <locale.h>
#include <stdio.h>

#include "../include/lmstock.h"
#include "../lmapi/lmapi.h"
#include "../lmapi/lmstrencode.h"

#include "lmice_eal_thread.h"

#include <thread>

#if defined(_WIN32)
#define WIN32_MEAN_AND_LEAN
#include <Windows.h>
#else
#include <dlfcn.h>
#endif

#include <cmath>
#include <iostream>
#include <vector>

extern int lm_run_file(const char* name);

int main(int argc, char** argv) {
  setlocale(LC_ALL, "");

  //test config lapi
  //lmapi_config_api* cfg = lmapi_config_open(nullptr, "hello");
  //printf("cfg ptr:%zu\tget_int%zu\n", reinterpret_cast<size_t>(cfg), reinterpret_cast<size_t>(cfg->get_int));
  //cfg->get_int("world");
  //printf("called get_int\n");
  //lmapi_config_close(cfg);
  //return 0;


  // test kdata pack
  //lmapi::lmkdatas kd;
  //std::cout << kd.size() << std::endl;

  //char buff[16];
  //kd.pack(buff, 16);
  //printf("%lu  %lu\n", *(size_t*)buff, *(size_t*)(buff + sizeof(size_t)));
  //kd.unpack(buff, 16);
  //printf("%lu  %lu\n", *(size_t*)buff, *(size_t*)(buff + sizeof(size_t)));

  //  auto api = new lmapi::lmapi();
  //  auto cfg = api->config_open("test_node.xml");
  //  auto plist = cfg->get_array_string("root.node.prop");
  //  for (size_t i = 0; i < plist.size(); ++i) {
  //    std::cout << plist[i] << std::endl;
  //  }

  //  delete cfg;
  //  plist.clear();
  //  cfg = api->config_open("test_prop.xml");
  //  plist = cfg->get_array_string("root.node.prop");
  //  for (size_t i = 0; i < plist.size(); ++i) {
  //    std::cout << plist[i] << std::endl;
  //  }
  //  delete cfg;
  //  delete api;
  //  //  return 0;

    //// test strencode
    //std::thread t([]() {
    //  auto encode = lmapi_strencode_capi();
    //  char* gbk = nullptr;
    //  char str[] = "hello world\n";
    //  size_t wstr_len = 0;
    //  size_t wstr_bytes = 0;
    //  wstr_len = encode->utf8_to_gbk(str, strlen(str) + 1, &gbk, &wstr_bytes);

    //  printf("hello world gbk %s size %lu\n", gbk, wstr_len);
    //});
    //t.join();

  //  auto encode = lmapi_strencode_capi();
  //  char str[] = "hello";
  //  wchar_t* wstr = nullptr;
  //  size_t wstr_len = 0;
  //  size_t wstr_bytes = 0;

  //  char str2[] = "welcome";
  //  wstr_len = encode->utf8_to_wstr(str2, strlen(str2) + 1, &wstr,
  //  &wstr_bytes); printf("%s to wstr %ls bytes: %lu len %lu\n", str2, wstr,
  //  wstr_bytes,
  //         wstr_len);

  //  wstr_len = encode->utf8_to_wstr(str, strlen(str) + 1, &wstr, &wstr_bytes);
  //  printf("%s to wstr %ls bytes: %lu len %lu\n", str, wstr, wstr_bytes,
  //         wstr_len);

  //  wstr_len = encode->utf8_to_wstr("hehao", 6, &wstr, &wstr_bytes);
  //  printf("%s to wstr %ls bytes: %lu len %lu\n", "hehao", wstr, wstr_bytes,
  //         wstr_len);

  //  free(wstr);
  //  return 0;

  if (argc != 3) {
    printf(
        "Usage:\n"
        "  [app] [dll file] [config file(json format)]\n"
        "\teg: lmagent factpr.dll test.json\n");
    return -1;
  }

  typedef void (*f_run)(const char* cfg);
#if defined(_WIN32)
  TCHAR buff[256];
  memset(buff, 0, sizeof(buff));
  GetCurrentDirectory(256, buff);
  printf("current folder is %ls\n", buff);

  HMODULE hmod = LoadLibraryA(argv[1]);
  DWORD err = GetLastError();
  f_run func = (f_run)GetProcAddress(hmod, "factor_run");
  if (func == nullptr) {
    printf("from [%p] get factor_run function %p as %d\n", hmod, func, err);
    return 1;
  }
#else
  void* hdll = dlopen(argv[1], RTLD_LAZY);
  f_run func = (f_run)dlsym(hdll, "factor_run");
  pthread_setname_np("LMAPI-AGENT");
#endif

  func(argv[2]);
  getchar();
  return 0;
}

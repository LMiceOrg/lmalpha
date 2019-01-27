#include "lmice_trace.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "lmice_eal_atomic.h"
#include "lmice_eal_time.h"

#if !defined(_DEBUG)
const int lmice_trace_debug_mode = 0;
#else
const int lmice_trace_debug_mode = 1;
#endif

#if defined(_WIN32)

lmice_trace_name_t lmice_trace_name[] = {
    {LMICE_TRACE_INFO, 10, 0, " INFO" /* light_green*/},
    {LMICE_TRACE_DEBUG, 11, 0, " DEBUG" /* light_cyan */},
    {LMICE_TRACE_WARNING, 14, 0, " WARNING" /*yellow*/},
    {LMICE_TRACE_ERROR, 12, 0, " ERROR" /*light_red*/},
    {LMICE_TRACE_CRITICAL, 13, 0, " CRITICAL" /* light_purple*/},
    {LMICE_TRACE_NONE, 7, 0, "NULL" /* white */},
    {LMICE_TRACE_TIME, 14, 0, "TIME" /* yellow*/}};

#else

lmice_trace_name_t lmice_trace_name[] = {
    {LMICE_TRACE_INFO, "INFO", "\033[1;32m" /* light_green*/},
    {LMICE_TRACE_DEBUG, "DEBUG", "\033[1;36m" /* light_cyan */},
    {LMICE_TRACE_WARNING, "WARNING", "\033[1;33m" /*yellow*/},
    {LMICE_TRACE_ERROR, "ERROR", "\033[1;31m" /*light_red*/},
    {LMICE_TRACE_CRITICAL, "CRITICAL", "\033[1;35m" /* light_purple*/},
    {LMICE_TRACE_NONE, "NULL", "\033[0m"},
    {LMICE_TRACE_TIME, "TIME", "\033[1;33m" /*light_brown*/}};

#endif

int64_t last_time = 0;
volatile int64_t trace_count = 0;

#define LMICE_TRACE_COLOR_TIME(tm)              \
  lmice_trace_name[LMICE_TRACE_TIME].color, tm, \
      lmice_trace_name[LMICE_TRACE_NONE].color

#define EAL_TRACE_0()                                                          \
  int64_t tr_cnt;                                                              \
  int64_t _trace_stm;                                                          \
  int _trace_ret;                                                              \
  time_t _trace_tm;                                                            \
  struct tm pt;                                                                \
  char _trace_current_time[80];                                                \
  char _trace_thread_name[32];                                                 \
  if (lmice_trace_debug_mode == 0 && type == LMICE_TRACE_DEBUG) return;        \
  memset(_trace_current_time, 0, sizeof(_trace_current_time));                 \
  get_system_time(&_trace_stm);                                                \
  tr_cnt = eal_fetch_and_add64(&trace_count, 1);                               \
  if ((tr_cnt % 100) == 0 || (_trace_stm - last_time) / 10000000LL > 60) {     \
    last_time = _trace_stm;                                                    \
    _trace_tm = _trace_stm / 10000000;                                         \
    gmtime_r(&_trace_tm, &pt);                                                 \
    sprintf(_trace_current_time, "%4d-%02d-%02d %02d:%02d:%02d.%07lld",        \
            pt.tm_year + 1900, pt.tm_mon + 1, pt.tm_mday, pt.tm_hour,          \
            pt.tm_min, pt.tm_sec, _trace_stm % 10000000);                      \
  } else {                                                                     \
    _trace_stm -= last_time;                                                   \
    pt.tm_sec = (_trace_stm / 10000000LL) % 60;                                \
    pt.tm_min = (_trace_stm / (60 * 10000000LL)) % 60;                         \
    pt.tm_hour = (_trace_stm / (60 * 60 * 10000000LL)) % 24;                   \
    if (pt.tm_hour == 0) {                                                     \
      if (pt.tm_min == 0) {                                                    \
        sprintf(_trace_current_time, "%02d.%07lld", pt.tm_sec,                 \
                _trace_stm % 10000000);                                        \
      } else {                                                                 \
        sprintf(_trace_current_time, "%02d:%02d.%07lld", pt.tm_min, pt.tm_sec, \
                _trace_stm % 10000000);                                        \
      }                                                                        \
    } else {                                                                   \
      sprintf(_trace_current_time, "%02d:%02d:%02d.%07lld", pt.tm_hour,        \
              pt.tm_min, pt.tm_sec, _trace_stm % 10000000);                    \
    }                                                                          \
  }                                                                            \
  /*change newline to space */                                                 \
  _trace_ret = pthread_getname_np(eal_gettid(), _trace_thread_name, 32);       \
  if (_trace_ret == 0) {                                                       \
    if (strlen(_trace_thread_name) == 0)                                       \
      _trace_ret = -1;                                                         \
    else                                                                       \
      _trace_ret = 0;                                                          \
  }

#if defined(_WIN32)
#define EAL_TRACE_WIN32()                             \
  printf(_trace_current_time);                        \
  LMICE_TRACE_COLOR_TAG3(type);                       \
  if (_trace_ret == 0) {                              \
    printf(":[%d:%s]", getpid(), _trace_thread_name); \
  } else {                                            \
    printf(":[%d:0x%llx]", getpid(), eal_gettid());   \
  }
#elif defined(__APPLE__) || defined(__linux__)
#define EAL_TRACE_UNIX()                                                \
  if (_trace_ret == 0) {                                                \
    printf("%s%s%s %s%s%s:[%d:%s]",                                     \
           LMICE_TRACE_COLOR_TIME(_trace_current_time),                 \
           LMICE_TRACE_COLOR_TAG3(type), getpid(), _trace_thread_name); \
  } else {                                                              \
    printf("%s%s%s %s%s%s:[%d:0x%lx]",                                  \
           LMICE_TRACE_COLOR_TIME(_trace_current_time),                 \
           LMICE_TRACE_COLOR_TAG3(type), getpid(),                      \
           (unsigned long)(void *)eal_gettid());                        \
  }
#endif
#if !defined(_WIN32)
void eal_trace_color_print_per_thread(int type) {
  EAL_TRACE_0();
#if defined(_WIN32)
  EAL_TRACE_WIN32();
#else /* UNIX */
  EAL_TRACE_UNIX();
#endif
}

void lmice_trace(int pid, int type, int64_t trace_stm, const char *thread_name,
                 const char *ctx) {
  int tm_sec = (trace_stm / 10000000LL) % 60;
  int tm_min = (trace_stm / (60 * 10000000LL)) % 60;
  int tm_hour = (trace_stm / (60 * 60 * 10000000LL)) % 24;
  int tm_nano100 = trace_stm % 10000000;
  char trace_current_time[32] = {0};
  int sec_pos = 0;
  int min_pos = 0;
  int hour_pos = 0;
  if (tm_hour == 0) {
    if (tm_min == 0) {
      sec_pos = 0;
    } else {
      sec_pos = 3;
    }
  } else {
    min_pos = 3;
    sec_pos = 6;
  }
  // hour
  trace_current_time[hour_pos + 0] = '0' + (tm_hour / 10);
  trace_current_time[hour_pos + 1] = '0' + (tm_hour % 10);
  trace_current_time[hour_pos + 2] = ':';
  // min
  trace_current_time[min_pos + 0] = '0' + (tm_min / 10);
  trace_current_time[min_pos + 1] = '0' + (tm_min % 10);
  trace_current_time[min_pos + 2] = ':';
  // sec
  trace_current_time[sec_pos + 0] = '0' + (tm_sec / 10);
  trace_current_time[sec_pos + 1] = '0' + (tm_sec % 10);
  trace_current_time[sec_pos + 2] = '.';
  trace_current_time[sec_pos + 3] = '0' + ((tm_nano100 % 10000000) / 1000000);
  trace_current_time[sec_pos + 4] = '0' + ((tm_nano100 % 1000000) / 100000);
  trace_current_time[sec_pos + 5] = '0' + ((tm_nano100 % 100000) / 10000);
  trace_current_time[sec_pos + 6] = '0' + ((tm_nano100 % 10000) / 1000);
  trace_current_time[sec_pos + 7] = '0' + ((tm_nano100 % 1000) / 100);
  trace_current_time[sec_pos + 8] = '0' + ((tm_nano100 % 100) / 10);
  trace_current_time[sec_pos + 9] = '0' + ((tm_nano100 % 10) / 1);

  printf(
      "%s%s%s"
      "%s%s%s"
      "[%d:%s]"
      "%s",
      LMICE_TRACE_COLOR_TIME(trace_current_time), LMICE_TRACE_COLOR_TAG3(type),
      pid, thread_name, ctx);
}
#endif

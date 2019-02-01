/** Copyright 2018, 2019 He Hao<hehaoslj@sina.com> */

#include <cstdarg>
#include <sstream>

#include "lmapi.h"

#include "lmice_eal_thread.h"
#include "lmice_eal_time.h"
#include "lmice_trace.h"

namespace lmapi {
struct console_internal {
  int type;
  std::string format;
  int64_t begin_time;
  unsigned int count;
  int pid;
  int64_t tid;
  std::string thread_name;
};

/** helper */

#define LMAPI_WIN_COLOR_TAG(type)                          \
  SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), \
                          lmice_trace_name[type].color);   \
  printf(lmice_trace_name[type].name);                     \
  SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), \
                          lmice_trace_name[LMICE_TRACE_NONE].color);

#define LMAPI_WIN_COLOR_TIME(t)                                      \
  SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),           \
                          lmice_trace_name[LMICE_TRACE_TIME].color); \
  printf(t);                                                         \
  SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),           \
                          lmice_trace_name[LMICE_TRACE_NONE].color);
inline void console_write(console_internal *con, const char *time_str, int type,
                          const char *buff) {
#if defined(_WIN32)
  switch (con->type) {
    case LMAPI_LOG_DATETIME:
      LMAPI_WIN_COLOR_TIME(time_str);
      LMAPI_WIN_COLOR_TAG(type);
      printf(":%s", buff);

      break;
    case LMAPI_LOG_PROCESS:
      LMAPI_WIN_COLOR_TAG(type);
      printf(":[%d]%s", con->pid, buff);
      break;
    case LMAPI_LOG_THREAD:
      LMAPI_WIN_COLOR_TAG(type);
      printf(":[%s]%s", con->thread_name.c_str(), buff);
      break;
    case LMAPI_LOG_DATETIME + LMAPI_LOG_PROCESS:
      LMAPI_WIN_COLOR_TIME(time_str);
      LMAPI_WIN_COLOR_TAG(type);
      printf(":[%d]%s", con->pid, buff);
      break;
    case LMAPI_LOG_DATETIME + LMAPI_LOG_THREAD:
      LMAPI_WIN_COLOR_TIME(time_str);
      LMAPI_WIN_COLOR_TAG(type);
      printf(":[%s]%s", con->thread_name.c_str(), buff);

      break;
    case LMAPI_LOG_PROCESS + LMAPI_LOG_THREAD:
      LMAPI_WIN_COLOR_TAG(type);
      printf(":[%d:%s]%s", con->pid, con->thread_name.c_str(), buff);
      break;
    default:
      LMAPI_WIN_COLOR_TIME(time_str);
      LMAPI_WIN_COLOR_TAG(type);
      printf(":[%d:%s]%s", con->pid, con->thread_name.c_str(), buff);
      break;
  }
#else
  std::string type_str = lmice_trace_name[type].color;
  type_str += lmice_trace_name[type].name;
  type_str += lmice_trace_name[LMICE_TRACE_NONE].color;
  // "\033[1;36mDEBUG\033[0m";
  switch (con->type) {
    case LMAPI_LOG_DATETIME:
      printf(con->format.c_str(), time_str, type_str.c_str(), buff);
      break;
    case LMAPI_LOG_PROCESS:
      printf(con->format.c_str(), type_str.c_str(), con->pid, buff);
      break;
    case LMAPI_LOG_THREAD:
      printf(con->format.c_str(), type_str.c_str(), con->thread_name.c_str(),
             buff);
      break;
    case LMAPI_LOG_DATETIME + LMAPI_LOG_PROCESS:
      printf(con->format.c_str(), time_str, type_str.c_str(), con->pid, buff);
      break;
    case LMAPI_LOG_DATETIME + LMAPI_LOG_THREAD:
      printf(con->format.c_str(), time_str, type_str.c_str(),
             con->thread_name.c_str(), buff);
      break;
    case LMAPI_LOG_PROCESS + LMAPI_LOG_THREAD:
      printf(con->format.c_str(), type_str.c_str(), con->pid,
             con->thread_name.c_str(), buff);
      break;
    default:
      printf(con->format.c_str(), time_str, type_str.c_str(), con->pid,
             con->thread_name.c_str(), buff);
      break;
  }
#endif
}
inline void time_string(console_internal *con, std::string &tstr) {
  //std::string format = "%4d-%02d-%02d %02d:%02d:%02d.%07lld";
  int64_t now;
  
  struct tm stack_pt;
  struct tm *pt = &stack_pt;
  char buff[64] = {'\0'};

  get_system_time(&now);
  
#if defined(_MSC_VER)
  SYSTEMTIME st_win;
  FILETIME ft_win;
  FileTimeToLocalFileTime((LPFILETIME)&now, &ft_win);
  FileTimeToSystemTime(&ft_win, &st_win);
  pt->tm_year = st_win.wYear;
  pt->tm_mon = st_win.wMonth;
  pt->tm_mday = st_win.wDay;
  pt->tm_hour = st_win.wHour;
  pt->tm_min = st_win.wMinute;
  pt->tm_sec = st_win.wSecond;

#else
  time_t tnow;
  tnow = now / 10000000;

  gmtime_r(&tnow, pt);
  pt->tm_year += 1900;
  pt->tm_mon += 1;
#endif

  if ((con->count % 100) != 1) {
    /** short format */
    now -= con->begin_time;
    pt->tm_sec = (now / 10000000LL) % 60;
    pt->tm_min = (now / (60 * 10000000LL)) % 60;
    pt->tm_hour = (now / (60 * 60 * 10000000LL)) % 24;

    if (pt->tm_hour == 0) {
      if (pt->tm_min == 0) {
        sprintf(buff, "%02d.%07lld", pt->tm_sec, now % 10000000);
      } else {
        sprintf(buff, "%02d:%02d.%07lld", pt->tm_min, pt->tm_sec,
                now % 10000000);
      }
    } else {
      sprintf(buff, "%02d:%02d:%02d.%07lld", pt->tm_hour, pt->tm_min,
              pt->tm_sec, now % 10000000);
    }
  } else {
    con->begin_time = now;
    sprintf(buff, "%4d-%02d-%02d %02d:%02d:%02d.%07lld", pt->tm_year ,
            pt->tm_mon, pt->tm_mday, pt->tm_hour, pt->tm_min, pt->tm_sec,
            now % 10000000);
  }

  tstr = buff;
}

inline void console_logging(console_internal *con, int type, const char *buff) {
  int64_t tid;
  std::string tname;
  std::string tstr;

  con->count++;
  if ((con->type == 0) || (con->type & LMAPI_LOG_DATETIME)) {
    time_string(con, tstr);
  }

  tid = reinterpret_cast<int64_t>(reinterpret_cast<void *>(eal_gettid()));
  if (tid != con->tid) {
    char name[128] = {0};
    std::ostringstream os;
    con->tid = tid;
    os << "0x" << std::hex << con->tid;
    con->thread_name = os.str();
    pthread_getname_np(eal_gettid(), name, 127);
    if (strlen(name) != 0) con->thread_name = name;
  }

  /*
  switch (con->type) {
    case LMAPI_LOG_DATETIME:
      printf(con->format.c_str(), tstr.c_str(), type.c_str(), buff);
      break;
    case LMAPI_LOG_PROCESS:
      printf(con->format.c_str(), type.c_str(), con->pid, buff);
      break;
    case LMAPI_LOG_THREAD:
      printf(con->format.c_str(), type.c_str(), con->thread_name.c_str(), buff);
      break;
    case LMAPI_LOG_DATETIME + LMAPI_LOG_PROCESS:
      printf(con->format.c_str(), tstr.c_str(), type.c_str(), con->pid, buff);
      break;
    case LMAPI_LOG_DATETIME + LMAPI_LOG_THREAD:
      printf(con->format.c_str(), tstr.c_str(), type.c_str(),
             con->thread_name.c_str(), buff);
      break;
    case LMAPI_LOG_PROCESS + LMAPI_LOG_THREAD:
      printf(con->format.c_str(), type.c_str(), con->pid,
             con->thread_name.c_str(), buff);
      break;
    default:
      printf(con->format.c_str(), tstr.c_str(), type.c_str(), con->pid,
             con->thread_name.c_str(), buff);
      break;
  }
  */
  console_write(con, tstr.c_str(), type, buff);
}

console::console(int tp) : pdata(NULL) {
  std::ostringstream os;
  console_internal *con;
  char name[128] = {0};

  con = new console_internal;
  con->type = tp;
  get_system_time(&con->begin_time);
  con->count = 0;
  con->pid = getpid();
  con->tid = reinterpret_cast<int64_t>(reinterpret_cast<void *>(eal_gettid()));
  os << "0x" << std::hex << con->tid;
  con->thread_name = os.str();
  pthread_getname_np(eal_gettid(), name, 127);
  if (strlen(name) != 0) con->thread_name = name;

  switch (tp) {
    case LMAPI_LOG_DATETIME:
      con->format = "\033[1;33m%s\033[0m %s:%s";
      break;
    case LMAPI_LOG_PROCESS:
      con->format = "%s:[%d]%s";
      break;
    case LMAPI_LOG_THREAD:
      con->format = "%s:[%s]%s";
      break;
    case LMAPI_LOG_DATETIME + LMAPI_LOG_PROCESS:
      con->format = "\033[1;33m%s\033[0m %s:[%d]%s";
      break;
    case LMAPI_LOG_DATETIME + LMAPI_LOG_THREAD:
      con->format = "\033[1;33m%s\033[0m %s:[%s]%s";
      break;
    case LMAPI_LOG_PROCESS + LMAPI_LOG_THREAD:
      con->format = "%s:[%d:%s]%s";
      break;
    default:
      con->format = "\033[1;33m%s\033[0m %s:[%d:%s]%s";
      break;
  };

  pdata = con;
}

console::~console() {
  console_internal *con;

  con = reinterpret_cast<console_internal *>(pdata);
  if (con) {
    delete con;
    pdata = NULL;
  }
}

void console::debug(const char *format, ...) {
  console_internal *con;

  std::string type = "\033[1;36mDEBUG\033[0m";

  std::va_list va;
  char buff[512] = {'\0'};
  va_start(va, format);
  vsprintf(buff, format, va);
  va_end(va);

  con = reinterpret_cast<console_internal *>(pdata);
  if (!con) return;

  console_logging(con, LMICE_TRACE_DEBUG, buff);
}

void console::info(const char *format, ...) {
  console_internal *con;

  std::string type = "\033[1;32mINFO\033[0m";

  std::va_list va;
  char buff[512] = {'\0'};
  va_start(va, format);
  vsprintf(buff, format, va);
  va_end(va);

  con = reinterpret_cast<console_internal *>(pdata);
  if (!con) return;

  console_logging(con, LMICE_TRACE_INFO, buff);
}

void console::warning(const char *format, ...) {
  console_internal *con;

  std::string type = "\033[1;33mWARNING\033[0m";

  std::va_list va;
  char buff[512] = {'\0'};
  va_start(va, format);
  vsprintf(buff, format, va);
  va_end(va);

  con = reinterpret_cast<console_internal *>(pdata);
  if (!con) return;

  console_logging(con, LMICE_TRACE_WARNING, buff);
}

void console::critical(const char *format, ...) {
  console_internal *con;

  std::string type = "\033[1;35mCRITICAL\033[0m";

  std::va_list va;
  char buff[512] = {'\0'};
  va_start(va, format);
  vsprintf(buff, format, va);
  va_end(va);

  con = reinterpret_cast<console_internal *>(pdata);
  if (!con) return;

  console_logging(con, LMICE_TRACE_CRITICAL, buff);
}

void console::error(const char *format, ...) {
  console_internal *con;

  std::string type = "\033[1;31mERROR\033[0m";

  std::va_list va;
  char buff[512] = {'\0'};
  va_start(va, format);
  vsprintf(buff, format, va);
  va_end(va);

  con = reinterpret_cast<console_internal *>(pdata);
  if (!con) return;

  console_logging(con, LMICE_TRACE_ERROR, buff);
}

}  // namespace lmapi

#include "lmapi.h"

#include <stdio.h>
#include <sys/stat.h>
#include <time.h>

namespace lmapi {

struct serial_internal {
  int type;
  std::string root;
  std::vector<std::string> instruments;
  int start_date;
  int end_date;
  time_t tm_begin;
  time_t tm_end;

  std::string get_file(const char* code, int date) {
    if (type == LMAPI_TICK_TYPE) {
      return get_rtick_file(code, date);
    } else
      return get_kdata_file(code, date);
  }

  std::string get_rtick_file(const char* code, int date) {
    std::string stype;
    std::string value;
#if defined(_WIN32)
    char sep = '\\';
#else
    char sep = '/';
#endif
    char buf[1024];

    // "\2019\0110\000070\20190110000070-L2_Tick"

    value = root;
    if (value.size() != 0) {
      if (*(value.end() - 1) != sep) {
        value += sep;
      }
    }
    int year = date / 10000;
    int yday = date % 10000;
    memset(buf, 0, sizeof(buf));
    snprintf(buf, sizeof(buf), "%s%04d%c%04d%c%s%c%08d%s-L2_tick",
             value.c_str(), year, sep, yday, sep, code, sep, date, code);

    value = buf;
    return value;
  }

  std::string get_kdata_file(const char* instrument, int tp) {
    std::string stype;
    std::string value;
#if defined(_WIN32)
    char sep = '\\';
#else
    char sep = '/';
#endif
    char buf[1024];

    (void)tp;

    //  root/instrument/instrument.Min_1.kdata

    value = root;
    if (value.size() != 0) {
      if (*(value.end() - 1) != sep) {
        value += sep;
      }
    }

    switch (type) {
      case lmapi_data_type::LMAPI_1MIN_TYPE:
        stype = "Min_1";
        break;
      case lmapi_data_type::LMAPI_5MIN_TYPE:
        stype = "Min_5";
        break;
      case lmapi_data_type::LMAPI_15MIN_TYPE:
        stype = "Min_15";
        break;
      case lmapi_data_type::LMAPI_30MIN_TYPE:
        stype = "Min_30";
        break;
      case lmapi_data_type::LMAPI_1HOUR_TYPE:
        stype = "Min_60";
        break;
      case lmapi_data_type::LMAPI_2HOUR_TYPE:
        stype = "Min_120";
        break;
    }
    memset(buf, 0, sizeof(buf));
    snprintf(buf, sizeof(buf), "%s%s%c%s.%s.kdata", value.c_str(), instrument,
             sep, instrument, stype.c_str());

    value = buf;
    return value;
  }
};

serial_dataset::serial_dataset(const std::string& root,
                               const std::vector<std::string>& instruments,
                               int tp, int start_date, int end_date) {
  serial_internal* se;
  se = new serial_internal;

  se->root = root;
  se->type = tp;
  se->start_date = start_date;
  se->end_date = end_date;
  se->instruments = instruments;

  struct tm begin;
  struct tm end;

  memset(&begin, 0, sizeof(struct tm));
  begin.tm_year = se->start_date / 10000;
  begin.tm_mon = (se->start_date % 10000) / 100;
  begin.tm_mday = (se->start_date % 100);
  se->tm_begin = mktime(&begin);

  memset(&end, 0, sizeof(struct tm));
  end.tm_year = se->end_date / 10000;
  end.tm_mon = (se->end_date % 10000) / 100;
  end.tm_mday = (se->end_date % 100);
  end.tm_hour = 1;
  se->tm_end = mktime(&end);

  pdata = se;
}

serial_dataset::~serial_dataset() {
  serial_internal* se;
  se = reinterpret_cast<serial_internal*>(pdata);
  delete se;
}

std::vector<lmtickdata> serial_dataset::get_tick(const std::string& code) {
  std::vector<lmtickdata> vec;
  serial_internal* se;
  se = reinterpret_cast<serial_internal*>(pdata);

  struct tm now;
  time_t tm_now = se->tm_begin;
  while (tm_now < se->tm_end) {
    gmtime_r(&tm_now, &now);
    int date;
    std::string tick_file;
    struct stat st;
    int ret;

    // 1. get file name
    date = (now.tm_year + 1900) * 10000 + (now.tm_mon + 1) * 100 + now.tm_mday;
    tick_file = se->get_file(code.c_str(), date);

    // 2. check file stat
    ret = stat(tick_file.c_str(), &st);
    if (ret == 0) {
      size_t num = st.st_size / sizeof(lmtickdata);
      if (st.st_size == num * sizeof(lmtickdata)) {
        FILE* fp = fopen(tick_file.c_str(), "rb");
        if (fp) {
          size_t pos = vec.size();
          vec.resize(vec.size() + num);

          fread(&vec[pos], 1, st.st_size, fp);
          fclose(fp);

        }  // fp
      }
    }

    // 3. next day
    tm_now += 86400;
  }  // while: tm_now - tm_end

  return vec;
}

std::vector<lmkdata> serial_dataset::get_kdata(const std::string& code) {
  std::vector<lmkdata> vec;
  serial_internal* se;
  se = reinterpret_cast<serial_internal*>(pdata);

  std::string data_file;

  // 1. get file name
  data_file = se->get_file(code.c_str(), se->type);
  // printf("data_file %s\n", data_file.c_str());

  struct stat st;
  int ret;

  // 2. check file stat
  ret = stat(data_file.c_str(), &st);
  if (ret == 0) {
    // printf("file size %lld %lu\n", st.st_size, sizeof(lmkdata));
    size_t num = st.st_size / sizeof(lmkdata);
    if (st.st_size == num * sizeof(lmkdata)) {
      FILE* fp = fopen(data_file.c_str(), "rb");
      if (fp) {
        size_t size;
        lmkdata data[128];
        do {
          size = fread(&data, 1, sizeof(lmkdata) * 128, fp);
          // printf("fread %lu %lu\n", size, size / sizeof(lmkdata));
          for (size_t i = 0; i < size / sizeof(lmkdata); ++i) {
            if (data[i].nDate >= se->start_date &&
                data[i].nDate <= se->end_date) {
              vec.push_back(data[i]);
            }
            //            if (i == 0)
            //              printf("date: %08d - %08d - %08d\n", se->start_date,
            //                     data[i].nDate, se->end_date);
          }
          if (feof(fp) != 0) break;
          if (size == 0) break;
        } while (true);
        fclose(fp);

      }  // fp
    }
  }

  return vec;
}

std::vector<std::vector<lmtickdata> > serial_dataset::get_ticks() {
  std::vector<std::vector<lmtickdata> > vec;
  serial_internal* se;
  se = reinterpret_cast<serial_internal*>(pdata);
  for (const auto& code : se->instruments) {
    std::vector<lmtickdata> tick_data;
    tick_data = get_tick(code);
    vec.push_back(tick_data);
  }

  return vec;
}

std::vector<std::vector<lmkdata> > serial_dataset::get_kdatas() {
  std::vector<std::vector<lmkdata> > vec;
  serial_internal* se;
  se = reinterpret_cast<serial_internal*>(pdata);
  for (const auto& code : se->instruments) {
    std::vector<lmkdata> tick_data;
    // printf("call get_kdata %s\n", code.c_str());
    tick_data = get_kdata(code);
    vec.push_back(tick_data);
  }

  return vec;
}

}  // namespace lmapi

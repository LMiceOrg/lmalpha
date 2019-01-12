/** Copyright 2018 He Hao<hehaoslj@sina.com> */
#include <algorithm>
#include <functional>
#include <string>

#include "lmapi.h"
#include "lmstock.h"

#include <call_thunk.h>

/** xml2 */
#include <libxml/parser.h>
#include <libxml/xmlmemory.h>

#if defined(WIN32) || defined(WIN64)
#define WIN32_MEAN_AND_LEAN
#include <Windows.h>

#elif defined(__APPLE__)
#include <mach-o/dyld.h>

#endif

#define LMAPI_CONFIG_NAME "lmalpha.xml"

static void GetExectuableFileName(char* buf, int* len) {
#if defined(WIN32) || defined(WIN64)
  GetModuleFileNameA(buf, len);

#elif defined(__APPLE__)
  unsigned int sz = *len;
  _NSGetExecutablePath(buf, &sz);
  *len = sz;

#elif defined(UNIX)
  if (readlink("/proc/self/exe", buf, *len) == -1) path = buf;

#elif defined(__FreeBSD__)
  int mib[4];
  mib[0] = CTL_KERN;
  mib[1] = KERN_PROC;
  mib[2] = KERN_PROC_PATHNAME;
  mib[3] = -1;
  sysctl(mib, 4, buf, sizeof(buf), NULL, 0);
  path = buf;

#elif defined(SUNOS)
  buf = getexecname();
#endif
}

static std::string get_rtick_file(const char* root, const char* instrument,
                                  int date) {
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
  snprintf(buf, sizeof(buf), "%s%04d%c%04d%c%s%c%08d%s-L2_tick", value.c_str(),
           year, sep, yday, sep, instrument, sep, date, instrument);

  value = buf;
  return value;
}
static std::string get_kdata_file(const char* root, const char* instrument,
                                  int type) {
  std::string stype;
  std::string value;
#if defined(_WIN32)
  char sep = '\\';
#else
  char sep = '/';
#endif
  char buf[1024];

  //  root/instrument/instrument.Min_1.kdata

  value = root;
  if (value.size() != 0) {
    if (*(value.end() - 1) != sep) {
      value += sep;
    }
  }

  switch (type) {
    case lmkdata::Min_1:
      stype = "Min_1";
      break;
    case lmkdata::Min_5:
      stype = "Min_5";
      break;
    case lmkdata::Min_15:
      stype = "Min_15";
      break;
    case lmkdata::Min_30:
      stype = "Min_30";
      break;
    case lmkdata::Min_60:
      stype = "Min_60";
      break;
    case lmkdata::Min_120:
      stype = "Min_120";
      break;
  }
  memset(buf, 0, sizeof(buf));
  snprintf(buf, sizeof(buf), "%s%s%c%s.%s.kdata", value.c_str(), instrument,
           sep, instrument, stype.c_str());

  value = buf;
  return value;
}

/** c++ part */

namespace lmapi {
/** lmapi */
lmapi::lmapi() {
  /** lmapi configuration and initialization */
  xmlDocPtr doc = 0;  //定义解析文档指针
  xmlNodePtr root = 0;  //定义结点指针(你需要它为了在各个结点间移动)
  xmlChar* value;  //临时字符串变量
  char path[1024];
  int len = 1024;
  std::string svalue;
  std::vector<std::string> rtick_list;
  std::vector<std::string> kdata_list;

  /** 1. Get app path */
  memset(path, 0, sizeof(path));
  GetExectuableFileName(path, &len);

  std::string str = path;
  size_t pos = str.rfind("/", str.size());
  if (pos > 0) {
    str = str.substr(0, pos + 1) + LMAPI_CONFIG_NAME;
  } else {
    str += LMAPI_CONFIG_NAME;
  }
  printf("exe path:%s pos:%lu\n", str.c_str(), pos);

  /** 2. to config path */
  doc = xmlReadFile(str.c_str(), "UTF-8", XML_PARSE_RECOVER);  //解析文件
  if (doc) root = xmlDocGetRootElement(doc);
  if (root) {
    xmlNodePtr node = root->xmlChildrenNode;  //获取子节点
    while (node != NULL) {
      if (!xmlStrcmp(node->name, (const xmlChar*)("KData"))) {
        //查找属性
        xmlAttrPtr attr = node->properties;
        while (attr != NULL) {
          if (0 == xmlStrcmp(attr->name, BAD_CAST "rootPath")) {
            value = xmlGetProp(node, BAD_CAST "rootPath");
            svalue = (char*)value;
            kdata_list.push_back(svalue);
            xmlFree(value);
            break;  // break while: attr

            attr = attr->next;
          }  //  while: attr
        }

      } else if (0 == xmlStrcmp(node->name, BAD_CAST "rawtick")) {
        //查找属性
        xmlAttrPtr attr = node->properties;
        while (attr != NULL) {
          if (0 == xmlStrcmp(attr->name, BAD_CAST "rootPath")) {
            value = xmlGetProp(node, BAD_CAST "rootPath");
            svalue = (char*)value;
            rtick_list.push_back(svalue);
            xmlFree(value);
            break;  // break while: attr

            attr = attr->next;
          }  //  while: attr
        }
      }

      node = node->next;
    }  // while: node
  }

  for (size_t i = 0; i < rtick_list.size(); ++i)
    printf("rawtick root = %s\n", rtick_list[i].c_str());
  for (size_t i = 0; i < kdata_list.size(); ++i)
    printf("kdata root = %s\n", kdata_list[i].c_str());
  printf("get 600123 120min kdata = ");
  svalue = get_kdata_file(kdata_list[0].c_str(), "600123", lmkdata::Min_120);
  printf("%s\n", svalue.c_str());
  printf("get 600123 20170412 rawtick = ");
  svalue = get_rtick_file(rtick_list[0].c_str(), "600123", 20170412);
  printf("%s\n", svalue.c_str());

  if (doc) xmlFreeDoc(doc);
}
lmapi::~lmapi() {}

/** config */
config* lmapi::config_open(const std::string& name) {
  config* cfg = new config(name);
  return cfg;
}

void lmapi::config_close(config* cfg) { delete cfg; }

/** console log */
console* lmapi::console_open(int tp) {
  console* con = new console(tp);
  return con;
}

void lmapi::console_close(console* con) { delete con; }

/** sql query */

//      "Driver=SQLite3;Database=/Users/hehao/work/"
//      "build-alpha-Desktop_Qt_5_10_1_clang_64bit-Debug/test.db";
#define LMAPI_DSN                           \
  "Driver={ODBC Driver 17 for SQL Server};" \
  "Server=192.168.2.106;"                   \
  "UId=jydb;Pwd=jydb;"                      \
  "Connect Timeout=2;";

sql_dataset* lmapi::sql_open(const std::string& query) {
  std::string dsn;
  dsn = LMAPI_DSN;
  sql_dataset* ds = new sql_dataset(dsn, query);
  return ds;
}

void lmapi::sql_close(sql_dataset* ds) { delete ds; }

/** result store */
factor_result* lmapi::result_open(const std::string& factor_name) {
  std::string dsn = LMAPI_DSN;
  factor_result* ds = new factor_result(dsn, factor_name);
  return ds;
}

void lmapi::result_close(factor_result* ds) { delete ds; }

}  // namespace lmapi

/* c part */

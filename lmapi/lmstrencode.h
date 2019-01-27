#ifndef LMAPI_LMSTRENCODE_H_
#define LMAPI_LMSTRENCODE_H_

/** Copyright 2018, 2019 He Hao<hehaoslj@sina.com> */

#ifdef __cplusplus

/** C++ only - Middle Level API */
#include <string>

namespace lmapi {
class strencode {
 public:
  friend class lmapi;
  std::wstring utf8_to_wstr(const std::string& from);
  std::string utf8_to_gbk(const std::string& from);

  std::string wstr_to_utf8(const std::wstring& from);
  std::string wstr_to_gbk(const std::wstring& from);

  std::string gbk_to_utf8(const std::string& from);
  std::wstring gbk_to_wstr(const std::string& from);

 private:
  strencode(void);
  void* pdata;
};

}  // namespace lmapi

extern "C" {
#endif

/** C only - Low Level API */

struct lmapi_strencode_api {
  size_t(*utf8_to_wstr)                     /* return wchar length */
      (const char* from, size_t from_bytes, /* from utf8, bytes */
       wchar_t** to, size_t* to_bytes);     /* to wchar, bytes */

  size_t(*utf8_to_gbk)                      /* return gbk length */
      (const char* from, size_t from_bytes, /* from utf8, bytes */
       char** to, size_t* to_bytes);        /* to gbk, bytes */

  size_t(*wstr_to_utf8)                        /* return utf8 length */
      (const wchar_t* from, size_t from_bytes, /* from wchar, bytes */
       char** to, size_t* to_bytes);           /* to utf8, bytes */

  size_t(*wstr_to_gbk)                         /* return gbk length */
      (const wchar_t* from, size_t from_bytes, /* from wchar, bytes */
       char** to, size_t* to_bytes);           /* to gbk, bytes */

  size_t(*gbk_to_wstr)                      /* return wchar length */
      (const char* from, size_t from_bytes, /* from gbk, bytes */
       wchar_t** to, size_t* to_bytes);     /* to wchar, bytes */

  size_t(*gbk_to_utf8)                      /* return utf8 length */
      (const char* from, size_t from_bytes, /* from gbk, bytes */
       char** to, size_t* to_bytes);        /* to utf8, bytes */
};

const struct lmapi_strencode_api* lmapi_strencode_capi(void);

#ifdef __cplusplus
}
#endif

#endif  // LMAPI_LMSTRENCODE_H_

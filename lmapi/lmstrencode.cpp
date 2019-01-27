/** Copyright 2018, 2019 He Hao<hehaoslj@sina.com> */

#include "lmstrencode.h"

#include <errno.h>
#include <iconv.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#define LMAPI_WCHAR_TYPE "UTF-16LE"
#else
#define LMAPI_WCHAR_TYPE "UTF-32LE"
#endif

#define LMAPI_GBK_TYPE "GBK"
#define LMAPI_UTF8_TYPE "UTF-8"

/** C++ only - Middle Level API */

namespace lmapi {

/** helper */
template <class T>
static inline size_t byte_size(const T& str) {
  typedef typename T::value_type char_type;
  return str.size() * sizeof(char_type);
}

template <class From>
static inline void str_encode_c(iconv_t ic, const From* from, size_t from_size,
                                char** out_buff, size_t* out_bytes) {
  typedef From from_type;

  char* inptr;
  char* outptr;
  char buff[1024];

  size_t inleft;
  size_t outleft;
  size_t buff_size = sizeof(buff);
  size_t ret = 0;
  size_t self_size = *out_bytes;

  *out_bytes = 0;

  inptr = reinterpret_cast<char*>(const_cast<from_type*>(from));
  inleft = from_size;

  // printf("in size %lu self %lu\n", inleft, self_size);

  // out_buff workable
  if (self_size >= inleft * 4) {
    // printf("out_buff workable\n");
    outptr = *out_buff;
    outleft = self_size;
    ret = iconv(ic, &inptr, &inleft, &outptr, &outleft);
    if (ret != static_cast<size_t>(-1)) {
      // printf("%lu\n", outleft);
      *out_bytes = self_size - outleft;
      // printf("out_bytes %lu\n", outleft);
    }

  } else if (buff_size >= inleft * 4) {
    // buff workable
    outptr = buff;
    outleft = sizeof(buff);
    ret = iconv(ic, &inptr, &inleft, &outptr, &outleft);
    // printf("ret %lu %lu\n", ret, inleft);
    if (ret != static_cast<size_t>(-1)) {
      size_t char_bytes = buff_size - outleft;

      if (self_size >= char_bytes) {
        memcpy(*out_buff, buff, char_bytes);
      } else {
        // printf("before realloc %p %lu\n", out_buff, char_bytes);
        *out_buff = reinterpret_cast<char*>(realloc(*out_buff, char_bytes));
        // printf("after realloc %p %lu\n", out_buff, char_bytes);

        memcpy(*out_buff, buff, char_bytes);
        // printf("copyed %ls\n", (wchar_t*)*out_buff);
      }

      *out_bytes = char_bytes;
    }
  } else {
    // realloc large buff
    buff_size = inleft * 4;

    *out_buff = reinterpret_cast<char*>(realloc(*out_buff, buff_size));

    outptr = *out_buff;
    outleft = buff_size;

    ret = iconv(ic, &inptr, &inleft, &outptr, &outleft);
    if (ret != static_cast<size_t>(-1)) {
      size_t char_bytes = (buff_size - outleft);
      *out_bytes = char_bytes;
    }
  }
}

template <class To, class From>
static inline void str_encode(iconv_t ic, const From& from, To* to) {
  char* inptr;
  char* outptr;
  char buff[1024];

  size_t inleft;
  size_t outleft;
  size_t buff_size;
  size_t ret;

  typedef typename From::value_type from_type;
  typedef typename To::value_type to_type;

  inptr = reinterpret_cast<char*>(const_cast<from_type*>(from.c_str()));
  outptr = buff;

  inleft = byte_size(from) + sizeof(from_type);
  // printf("in size %lu\n", inleft);
  outleft = sizeof(buff);

  buff_size = sizeof(buff);

  if (outleft > inleft * 4) {
    // printf("inleft %lu, outleft %lu\n", inleft, outleft);
    ret = iconv(ic, &inptr, &inleft, &outptr, &outleft);
    // int err = errno;
    // printf("ret %lu err:%d\n", ret, err);

    if (ret != static_cast<size_t>(-1)) {
      size_t char_size = (sizeof(buff) - outleft) / sizeof(to_type);
      to_type* char_begin = reinterpret_cast<to_type*>(buff);
      // to.resize(char_size);

      // to.replace(to.begin(), to.end(), char_begin, char_begin + char_size);
      to->insert(to->begin(), char_begin, char_begin + char_size);

      printf("converted char size %lu  final size %lu\n", char_size,
             to->size());
    }

  } else {
    buff_size = inleft * 4;
    outleft = buff_size;

    char* large_buff = new char[buff_size];
    outptr = large_buff;

    ret = iconv(ic, &inptr, &inleft, &outptr, &outleft);
    if (ret != static_cast<size_t>(-1)) {
      size_t char_size = (buff_size - outleft) / sizeof(to_type);
      to_type* char_begin = reinterpret_cast<to_type*>(large_buff);
      to->insert(to->begin(), char_begin, char_begin + char_size);
    }

    delete[] large_buff;
  }
}

struct strencode_internal {
  iconv_t utf8_wstr;
  iconv_t utf8_gbk;
  iconv_t wstr_utf8;
  iconv_t wstr_gbk;
  iconv_t gbk_wstr;
  iconv_t gbk_utf8;

  strencode_internal(void) {
    // iconv_t err = reinterpret_cast<iconv_t>(-1);
    const char utf8[] = LMAPI_UTF8_TYPE;
    const char gbk[] = LMAPI_GBK_TYPE;
    const char wstr[] = LMAPI_WCHAR_TYPE;

    utf8_wstr = iconv_open(utf8, wstr);
    utf8_gbk = iconv_open(utf8, gbk);

    wstr_utf8 = iconv_open(wstr, utf8);
    wstr_gbk = iconv_open(wstr, gbk);

    gbk_wstr = iconv_open(gbk, wstr);
    gbk_utf8 = iconv_open(gbk, utf8);
  }

  ~strencode_internal() {
    iconv_close(utf8_wstr);
    iconv_close(utf8_gbk);

    iconv_close(wstr_utf8);
    iconv_close(wstr_gbk);

    iconv_close(gbk_wstr);
    iconv_close(gbk_utf8);
  }

  int utf8_to_wstr(const char* from, wchar_t* to, size_t to_bytes) {
    std::string sfrom = from;
    std::wstring sto;
    str_encode(utf8_wstr, sfrom, &sto);

    size_t to_len = sto.size() * sizeof(wchar_t);
    if (to_bytes == 0 || to == NULL) {
      return to_len;
    } else if (to_len <= to_bytes) {
      memcpy(reinterpret_cast<char*>(to), sto.c_str(), to_len);
    }
    return to_len;
  }
};

strencode::strencode(void) {
  strencode_internal* si = new strencode_internal();
  pdata = si;
}

std::wstring strencode::utf8_to_wstr(const std::string& from) {
  std::wstring to;

  strencode_internal* si;

  si = reinterpret_cast<strencode_internal*>(pdata);
  str_encode(si->utf8_wstr, from, &to);

  return to;
}

std::string strencode::utf8_to_gbk(const std::string& from) {
  std::string to;
  strencode_internal* si;

  si = reinterpret_cast<strencode_internal*>(pdata);
  str_encode(si->utf8_gbk, from, &to);

  return to;
}

std::string strencode::wstr_to_utf8(const std::wstring& from) {
  std::string to;
  strencode_internal* si;

  si = reinterpret_cast<strencode_internal*>(pdata);
  str_encode(si->wstr_utf8, from, &to);

  return to;
}
std::string strencode::wstr_to_gbk(const std::wstring& from) {
  std::string to;
  strencode_internal* si;

  si = reinterpret_cast<strencode_internal*>(pdata);
  str_encode(si->wstr_gbk, from, &to);

  return to;
}

std::string strencode::gbk_to_utf8(const std::string& from) {
  std::string to;
  strencode_internal* si;

  si = reinterpret_cast<strencode_internal*>(pdata);
  str_encode(si->gbk_utf8, from, &to);

  return to;
}
std::wstring strencode::gbk_to_wstr(const std::string& from) {
  std::wstring to;
  strencode_internal* si;

  si = reinterpret_cast<strencode_internal*>(pdata);
  str_encode(si->gbk_wstr, from, &to);

  return to;
}

}  // namespace lmapi

/** 线程本地存储: 字符转换状态 */
class lmapi_tls_icodes {
 public:
  lmapi_tls_icodes() {
    tls_ic[0] = iconv_open(LMAPI_WCHAR_TYPE, LMAPI_UTF8_TYPE);
    tls_ic[1] = iconv_open(LMAPI_GBK_TYPE, LMAPI_UTF8_TYPE);
    tls_ic[2] = iconv_open(LMAPI_UTF8_TYPE, LMAPI_WCHAR_TYPE);
    tls_ic[3] = iconv_open(LMAPI_GBK_TYPE, LMAPI_WCHAR_TYPE);
    tls_ic[4] = iconv_open(LMAPI_WCHAR_TYPE, LMAPI_GBK_TYPE);
    tls_ic[5] = iconv_open(LMAPI_UTF8_TYPE, LMAPI_GBK_TYPE);
    printf("thread init tls icodes\n");
  }
  ~lmapi_tls_icodes() {
    for (size_t i = 0; i < sizeof(tls_ic) / sizeof(iconv_t); ++i) {
      iconv_close(tls_ic[i]);
    }
    printf("thread delete tls icodes\n");
  }
  inline iconv_t operator[](size_t pos) const { return tls_ic[pos]; }

 private:
  iconv_t tls_ic[6];
};
//__thread iconv_t lmapi_tls_ic[6];
static thread_local lmapi_tls_icodes lmapi_tls_ic;

static size_t lmapi_utf8_to_wstr(const char* from, size_t from_bytes,
                                 wchar_t** to, size_t* to_bytes) {
  char** c_to = reinterpret_cast<char**>(to);
  lmapi::str_encode_c(lmapi_tls_ic[0], from, from_bytes, c_to, to_bytes);

  return *to_bytes / sizeof(wchar_t);
}

static size_t lmapi_utf8_to_gbk(const char* from, size_t from_bytes, char** to,
                                size_t* to_bytes) {
  char** c_to = reinterpret_cast<char**>(to);
  lmapi::str_encode_c(lmapi_tls_ic[1], from, from_bytes, c_to, to_bytes);

  return *to_bytes / sizeof(char);
}

static size_t lmapi_wstr_to_utf8(const wchar_t* from, size_t from_bytes,
                                 char** to, size_t* to_bytes) {
  char** c_to = reinterpret_cast<char**>(to);
  lmapi::str_encode_c(lmapi_tls_ic[2], from, from_bytes, c_to, to_bytes);

  return *to_bytes / sizeof(char);
}
static size_t lmapi_wstr_to_gbk(const wchar_t* from, size_t from_bytes,
                                char** to, size_t* to_bytes) {
  char** c_to = reinterpret_cast<char**>(to);
  lmapi::str_encode_c(lmapi_tls_ic[3], from, from_bytes, c_to, to_bytes);

  return *to_bytes / sizeof(char);
}

static size_t lmapi_gbk_to_wstr(const char* from, size_t from_bytes,
                                wchar_t** to, size_t* to_bytes) {
  char** c_to = reinterpret_cast<char**>(to);

  lmapi::str_encode_c(lmapi_tls_ic[4], from, from_bytes, c_to, to_bytes);

  return *to_bytes / sizeof(wchar_t);
}

static size_t lmapi_gbk_to_utf8(const char* from, size_t from_bytes, char** to,
                                size_t* to_bytes) {
  char** c_to = reinterpret_cast<char**>(to);

  lmapi::str_encode_c(lmapi_tls_ic[5], from, from_bytes, c_to, to_bytes);

  return *to_bytes / sizeof(char);
}

/** 全局静态变量 lmapi_strencode_c_api,  Low Level API 字符转换函数指针结构体 */
static struct lmapi_strencode_api lmapi_strencode_c_api {
  lmapi_utf8_to_wstr, lmapi_utf8_to_gbk, lmapi_wstr_to_utf8, lmapi_wstr_to_gbk,
      lmapi_gbk_to_wstr, lmapi_gbk_to_utf8
};

/** 返回 字符转换函数指针结构体 */
const struct lmapi_strencode_api* lmapi_strencode_capi(void) {
  return &lmapi_strencode_c_api;
}

/** Copyright 2018, 2019 He Hao<hehaoslj@sina.com> */

#include <stdlib.h>
#include <string.h>

#include "lmiconv.h"

#if defined(_MSC_VER) && defined(LM_NO_STD_ICONV)
#define WIN32_MEAN_AND_LEAN
#include <Windows.h>

enum win32_codepage_e { DIR_TO_WCHAR, DIR_TO_ANSI };
struct win32_iconv_t {
  char from[32];
  char to[32];
  int codepage;
  int direction;
};

iconv_t iconv_open(const char* to, const char* from) {
  win32_iconv_t* cs =
      reinterpret_cast<win32_iconv_t*>(malloc(sizeof(win32_iconv_t)));
  size_t to_sz = strlen(to);
  to_sz = to_sz > sizeof(to) ? sizeof(to) - 1 : to_sz;
  size_t from_sz = strlen(from);
  from_sz = from_sz > sizeof(from) ? sizeof(from) - 1 : from_sz;

  memset(cs, 0, sizeof(win32_iconv_t));
  memcpy(cs->to, to, to_sz);
  memcpy(cs->from, from, from_sz);

  if (strcmp(from, "UTF-16LE") == 0 || strcmp(from, "UTF-16") == 0 ||
      strcmp(from, "UTF16") == 0) {
    cs->direction = DIR_TO_ANSI;
  } else {
    cs->direction = DIR_TO_WCHAR;
  }

  if (strcmp(to, "GBK") == 0 || strcmp(to, "GB2312") == 0 ||
      strcmp(to, "GB18030") == 0 || strcmp(to, "ANSI") == 0) {
    cs->codepage = CP_OEMCP;
  } else {
    cs->codepage = CP_UTF8;
  }

  return reinterpret_cast<iconv_t>(cs);
}

size_t iconv(iconv_t codecs, char** in, size_t* in_size, char** out,
             size_t* out_size) {
  win32_iconv_t* cs;

  cs = reinterpret_cast<win32_iconv_t*>(codecs);

  if (cs->direction == DIR_TO_WCHAR) {
    wchar_t* wc_out = reinterpret_cast<wchar_t*>(*out);
    int out_len;
    out_len = MultiByteToWideChar(cs->codepage,   /* code page */
                                  MB_PRECOMPOSED, /* option */
                                  *in,            /* in string */
                                  -1,             /* in size */
                                  NULL,           /* out wchar string */
                                  -1              /* out size */
    );
    if (out_len > *out_size) out_len = *out_size;
    MultiByteToWideChar(cs->codepage,   /* code page */
                        MB_PRECOMPOSED, /* option */
                        *in,            /* in string */
                        -1,             /* in size */
                        wc_out,         /* out wchar string */
                        out_len         /* out size */
    );

  } else {
    const wchar_t* wc_in = reinterpret_cast<const wchar_t*>(*in);
    int out_len;
    out_len = WideCharToMultiByte(cs->codepage,   /* code page */
                                  WC_DEFAULTCHAR, /* option */
                                  wc_in,          /* in wchar string */
                                  -1,             /* in length */
                                  NULL,           /* out string */
                                  0,              /* out length */
                                  NULL, FALSE);
    if (out_len > *out_size) out_len = *out_size;

    WideCharToMultiByte(cs->codepage,   /* code page */
                        WC_DEFAULTCHAR, /* option */
                        wc_in,          /* in wchar string */
                        -1,             /* in length */
                        *out,           /* out string */
                        out_len,        /* out length */
                        NULL, FALSE);
    *out_size -= out_len;
    return out_len;
  }
}
void iconv_close(iconv_t codecs) {
  win32_iconv_t* cs;

  cs = reinterpret_cast<win32_iconv_t*>(codecs);
  free(cs);
}

#endif

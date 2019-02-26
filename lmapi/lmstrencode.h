#ifndef LMAPI_LMSTRENCODE_H_
#define LMAPI_LMSTRENCODE_H_

/** Copyright 2018, 2019 He Hao<hehaoslj@sina.com> */

#ifdef __cplusplus

/** C++ only - Middle Level API */

extern "C" {
#endif

    /** C only - Low Level API */

     /* 字符转换 LLAPI */
    struct lmapi_strencode_api {
        size_t(*utf8_to_wstr)                     /* return wchar length */
            (const char* from, size_t from_bytes, /* from utf8, bytes */
                char** to, size_t* to_bytes);     /* to wchar, bytes */

        size_t(*utf8_to_gbk)                      /* return gbk length */
            (const char* from, size_t from_bytes, /* from utf8, bytes */
                char** to, size_t* to_bytes);        /* to gbk, bytes */

        size_t(*wstr_to_utf8)                        /* return utf8 length */
            (const char* from, size_t from_bytes, /* from wchar, bytes */
                char** to, size_t* to_bytes);           /* to utf8, bytes */

        size_t(*wstr_to_gbk)                         /* return gbk length */
            (const char* from, size_t from_bytes, /* from wchar, bytes */
                char** to, size_t* to_bytes);           /* to gbk, bytes */

        size_t(*gbk_to_wstr)                      /* return wchar length */
            (const char* from, size_t from_bytes, /* from gbk, bytes */
                char** to, size_t* to_bytes);     /* to wchar, bytes */

        size_t(*gbk_to_utf8)                      /* return utf8 length */
            (const char* from, size_t from_bytes, /* from gbk, bytes */
                char** to, size_t* to_bytes);        /* to utf8, bytes */

        size_t(*utf8_to_utf16) /* return utf16le length */
            (const char* from, size_t from_bytes, /* from utf8, bytes */
                char** to, size_t* to_bytes); /* to utf16le, bytes */

        size_t(*utf16_to_utf8) /* return utf8 length */
            (const char* from, size_t from_bytes, /* from utf16, bytes */
                char** to, size_t* to_bytes); /* to utf8, bytes */

        size_t(*wchar_to_utf16) /* return utf16le length */
            (const char* from, size_t from_bytes, /* from wchar, bytes */
                char** to, size_t* to_bytes); /* to utf16, bytes */

        size_t(*utf16_to_wchar) /* return wchar length */
            (const char* from, size_t from_bytes, /* from utf16le, bytes */
                char** to, size_t* to_bytes); /* to wchar, bytes */
    };

    /* 获取字符转换API */
    const struct lmapi_strencode_api* lmapi_strencode_capi(void);

#ifdef __cplusplus
}
#endif

#endif  // LMAPI_LMSTRENCODE_H_

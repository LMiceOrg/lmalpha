#include <stdint.h>

#include "lmice_logger.h"

#include "lmice_eal_time.h"
#include "lmice_eal_thread.h"

#include "bson.h"


forceinline int eal_int64_to_hexstring(uint64_t p, char* buf)
{
    const char hex_string[]={'0','1','2','3','4','5','6','7','8','9', 'A', 'B', 'C', 'D', 'E', 'F'};
    char* pd = buf+15;
    int i = 0;

    for(i=0; i<8; ++i)
    {
        int v = p & 0xff;

        *(pd-1) = hex_string[ v >> 4];
        *(pd) = hex_string[ v & 0xf];
        pd-=2;
        p = p >> 8;
        if(p == 0)
            break;
    }
    return i+1;
}

forceinline void eal_int_to_decstring(int p, int w, char* buf)
{
    const char dec_string[]={'0','1','2','3','4','5','6','7','8','9'};
    char* pd = buf+w;
    int i = 0;

    for(i=0; i<w; ++i)
    {
        p = p % 10;
        *pd = dec_string[p];
        --pd;
        p = p / 10;
    }
}

forceinline void eal_get_localtime(char*buf)
{
    int64_t log_stm;
    time_t log_tm;
    struct tm log_pt;

    get_system_time(&log_stm);
    log_tm =log_stm / 10000000LL;
    localtime_r(&log_tm, &log_pt);

/*
    snprintf(log_current_time, 28, "%4d-%02d-%02d %02d:%02d:%02d.%07lld",
            log_pt.tm_year+1900, log_pt.tm_mon+1, log_pt.tm_mday,
            log_pt.tm_hour, log_pt.tm_min, log_pt.tm_sec,
            log_stm%10000000);
*/
    eal_int_to_decstring(log_pt.tm_year+1900,   4, buf);
    *(buf+4)='-';
    eal_int_to_decstring(log_pt.tm_mon+1,       2, buf+5);
    *(buf+7)='-';
    eal_int_to_decstring(log_pt.tm_mday,        2, buf+8);
    *(buf+10)=' ';

    eal_int_to_decstring(log_pt.tm_hour,        2, buf+11);
    *(buf+13)=' ';
    eal_int_to_decstring(log_pt.tm_min,         2, buf+14);
    *(buf+16)=' ';
    eal_int_to_decstring(log_pt.tm_sec,         2, buf+17);
    *(buf+19)=' ';
    eal_int_to_decstring(log_stm%10000000,      7, buf+20);
    *(buf+28)='\0';

}

forceinline void eal_get_threadname(char*buf)
{
    eal_tid_t tid;
    int ret;
    tid = eal_gettid();
    ret = pthread_getname_np(eal_gettid(), buf, 32);
    if(buf[0] == '\0') {
        char* p = buf+2;
        buf[0]='0';
        buf[1]='x';
        ret = eal_int64_to_hexstring((uint64_t)tid, p);
        if(ret <8)
            memmove(p, p+(8-ret)*2, ret*2);
    }
}

forceinline void eal_log(const uint8_t* pdata, int size)
{
}

void lmice_log(lmice_logger_type_t log_type, const char* format, ...)
{
    va_list args;
    char log_current_time[28];
    char log_thread_name[32];
    eal_pid_t pid;

    eal_get_localtime(log_current_time);
    eal_get_threadname(log_thread_name);
    pid = getpid();

    if(log_type % lmice_logger_bson)
    {
        int size;
        const char* pdata;
        const char* name=format;
        bson_t bson;
        /* Bson format log */
        va_start(args, format);
        size = va_arg(args, int);
        pdata = va_arg(args, const char*);
        va_end(args);

        bson_init(&bson);

        BSON_APPEND_BINARY(&bson, "data", BSON_SUBTYPE_BINARY, (const uint8_t*)pdata, size);
        BSON_APPEND_UTF8(&bson, "name", name);
        BSON_APPEND_INT32(&bson, "size", size);

        BSON_APPEND_UTF8(&bson, "time", log_current_time);
        BSON_APPEND_UTF8(&bson, "thread", log_thread_name);
        BSON_APPEND_INT32(&bson, "process", pid);
        BSON_APPEND_INT32(&bson, "type", log_type);

        eal_log( bson_get_data(&bson), bson.len);

        bson_destroy(&bson);

    }
    else
    {
        int pos;
        char data[512];
        pos = sprintf(data, "%s %d:[%d:%s]", log_current_time, log_type, pid, log_thread_name);
        va_start(args, format);
        pos += vsnprintf(data+pos, 511-pos, format, args);
        va_end(args);

        eal_log( (const uint8_t*)data, pos+1);
    }
}

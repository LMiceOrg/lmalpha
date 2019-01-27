#if defined(_WIN32) && !defined(__GNUC__)
#define uint32_t unsigned int
#define int32_t int

#define uint16_t unsigned short
#define int16_t short

#define int8_t char
#define uint8_t unsigned char

#define int64_t __int64
#define uint64_t unsigned __int64

#endif

#include "lmice_eal_spinlock.h"

#include "lmice_eal_atomic.h"
#include "lmice_eal_time.h"

#define LOCK_LOOP_COUNT         20000000

int eal_spin_trylock(volatile spinlock_t* lock)
{
    int ret = 1;
    uint64_t cnt = 0;
    int64_t locked = 1;
    int sleep_times = 0;

    do {
        locked = eal_compare_and_swap32(lock, 0, 1);
        if(locked == 0)
        {
            ret = 0;
            break;
        }
        cnt ++;
        if(cnt >= LOCK_LOOP_COUNT)
        {
            usleep(1);
            sleep_times++;
            cnt = 0;
        }
    }while(sleep_times < 10);

    return ret;
}

int eal_spin_lock(volatile spinlock_t* lock)
{
    int64_t locked = 1;
    while(locked != 0)
    {
        locked = eal_compare_and_swap32(lock, 0, 1);
    }
    return 0;
}

int eal_spin_unlock(volatile spinlock_t *lock)
{
    eal_fetch_and_sub32(lock, 1);
    return 0;
}

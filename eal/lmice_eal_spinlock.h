#ifndef LMICE_EAL_SPINLOCK_H
#define LMICE_EAL_SPINLOCK_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32) && !defined(__GNUC__)
#define int64_t __int64
#define spinlock_t long
#else
#include <stdint.h>
#define spinlock_t long
#endif

int eal_spin_trylock(volatile spinlock_t *lock);
int eal_spin_lock(volatile spinlock_t *lock);
int eal_spin_unlock(volatile spinlock_t *lock);


#ifdef __cplusplus
}
#endif
#endif /** LMICE_EAL_SPINLOCK_H */

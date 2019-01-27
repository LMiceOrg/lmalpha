#ifndef LMICE_EAL_RWLOCK_H
#define LMICE_EAL_RWLOCK_H

/** John Mellor-Crummey 和 Michael Scott 提出，
 * 详见"Scalable Reader-Writer Synchronization for Shared-Memory
 * Multiprocessors"，
 */
#include <errno.h>
#include "lmice_eal_atomic.h"
#include "lmice_eal_common.h"

union lmice_eal_read_write_ticket_u {
  unsigned u;
  unsigned short us;
  __extension__ struct {
    unsigned char write;
    unsigned char read;
    unsigned char users;
  } s;
};

typedef union lmice_eal_read_write_ticket_u lmice_rwlock_t;

forceinline void eal_rwlock_wlock(lmice_rwlock_t *l) {
  unsigned me = eal_atomic_xadd(&l->u, (1 << 16));
  unsigned char val = me >> 16;

  while (val != l->s.write) cpu_relax();
}

forceinline void eal_rwlock_wunlock(lmice_rwlock_t *l) {
  lmice_rwlock_t t = *l;

  barrier();

  t.s.write++;
  t.s.read++;

  *(unsigned short *)l = t.us;
}

forceinline int eal_rwlock_wtrylock(lmice_rwlock_t *l) {
  unsigned me = l->s.users;
  unsigned char menew = me + 1;
  unsigned read = l->s.read << 8;
  unsigned cmp = (me << 16) + read + me;
  unsigned cmpnew = (menew << 16) + read + me;

  if (eal_atomic_cmpxchg(&l->u, cmp, cmpnew) == cmp) return 0;

  return EBUSY;
}

forceinline void eal_rwlock_rlock(lmice_rwlock_t *l) {
  unsigned me = eal_atomic_xadd(&l->u, (1 << 16));
  unsigned char val = me >> 16;

  while (val != l->s.read) cpu_relax();
  l->s.read++;
}

forceinline void eal_rwlock_runlock(lmice_rwlock_t *l) {
  eal_atomic_inc(&l->s.write);
}

forceinline int eal_rwlock_rtrylock(lmice_rwlock_t *l) {
  unsigned me = l->s.users;
  unsigned write = l->s.write;
  unsigned char menew = me + 1;
  unsigned cmp = (me << 16) + (me << 8) + write;
  unsigned cmpnew = ((unsigned)menew << 16) + (menew << 8) + write;

  if (eal_atomic_cmpxchg(&l->u, cmp, cmpnew) == cmp) return 0;

  return EBUSY;
}

#endif /** LMICE_EAL_RWLOCK_H */

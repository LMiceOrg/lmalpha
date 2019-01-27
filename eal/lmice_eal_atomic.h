#ifndef LMICE_EAL_ATOMIC_H
#define LMICE_EAL_ATOMIC_H

#include "lmice_eal_common.h"

#if defined(__GNUC__) && __GNUC__ > 3

/** Only for performance analysis purpose, DONT USE IT when work */
forceinline void eal_atomic_cycle_warmup(unsigned *high, unsigned *low) {
  __asm__ volatile(
      "CPUID\n\t"
      "RDTSC\n\t"
      "mov %%edx, %0\n\t"
      "mov %%eax, %1\n\t"
      : "=r"(*high), "=r"(*low)::"%rax", "%rbx", "%rcx", "%rdx");
  __asm__ volatile(
      "RDTSCP\n\t"
      "mov %%edx, %0\n\t"
      "mov %%eax, %1\n\t"
      "CPUID\n\t"
      : "=r"(*high), "=r"(*low)::"%rax", "%rbx", "%rcx", "%rdx");
  __asm__ volatile(
      "CPUID\n\t"
      "RDTSC\n\t"
      "mov %%edx, %0\n\t"
      "mov %%eax, %1\n\t"
      : "=r"(*(high + 1)), "=r"(*(low + 1))::"%rax", "%rbx", "%rcx", "%rdx");
  __asm__ volatile(
      "RDTSCP\n\t"
      "mov %%edx, %0\n\t"
      "mov %%eax, %1\n\t"
      "CPUID\n\t"
      : "=r"(*(high + 1)), "=r"(*(low + 1))::"%rax", "%rbx", "%rcx", "%rdx");
}
forceinline void eal_atomic_cycle_begin(unsigned *high, unsigned *low) {
  __asm__ __volatile__(
      "CPUID\n\t"
      "RDTSC\n\t"
      "mov %%edx, %0\n\t"
      "mov %%eax, %1\n\t"
      : "=r"(*high), "=r"(*low)::"%rax", "%rbx", "%rcx", "%rdx");
}
forceinline void eal_atomic_cycle_end(unsigned *high, unsigned *low) {
  __asm__ __volatile__(
      "RDTSCP\n\t"
      "mov %%edx, %0\n\t"
      "mov %%eax, %1\n\t"
      "CPUID\n\t"
      : "=r"(*high), "=r"(*low)::"%rax", "%rbx", "%rcx", "%rdx");
}

forceinline unsigned long long eal_atomic_cycle_value(unsigned *high,
                                                      unsigned *low) {
  return (((unsigned long long)high[1] << 32) + low[1]) -
         (((unsigned long long)high[0] << 32) + low[0]);
}

/** Pause instruction to prevent excess processor bus usage */
#define cpu_relax() \
  { __asm__ volatile("pause\n" ::: "memory"); }

#define cpu_nop() \
  { __asm__ volatile("nop" ::: "memory"); }

/** Compile read - write barrier */
#define barrier() \
  { __asm__ volatile("" : : : "memory"); }

/** Atomic exchange (of various sizes) */
forceinline void *eal_atomic_xchg64(void *ptr, void *x) {
  unsigned long long result = 0;
  __asm__ __volatile__("xchgq %0,%1"
                       : "=r"(result)
                       : "m"(*(volatile long long *)ptr),
                         "0"((unsigned long long)x)
                       : "memory");

  return (void *)result;
}

forceinline unsigned eal_atomic_xchg32(void *ptr, unsigned x) {
  unsigned result = 0;
  __asm__ __volatile__("xchgl %0,%1"
                       : "=r"(result)
                       : "m"(*(volatile unsigned *)ptr), "0"(x)
                       : "memory");

  return result;
}

forceinline unsigned short eal_atomic_xchg16(void *ptr, unsigned short x) {
  unsigned short result;
  __asm__ __volatile__("xchgw %0,%1"
                       : "=r"(result)
                       : "m"(*(volatile unsigned short *)ptr), "0"(x)
                       : "memory");

  return result;
}

/** Test and set a bit */
forceinline char eal_atomic_bitsetandtest(void *ptr, int x) {
  char out;
  __asm__ __volatile__(
      "lock; bts %2,%1\n"
      "sbb %0,%0\n"
      : "=r"(out), "=m"(*(volatile long long *)ptr)
      : "Ir"(x)
      : "memory");

  return out;
}

#define eal_atomic_xadd(P, V) __sync_fetch_and_add((P), (V))
#define eal_atomic_cmpxchg(P, O, N) __sync_val_compare_and_swap((P), (O), (N))
#define eal_atomic_inc(P) __sync_add_and_fetch((P), 1)
#define eal_atomic_dec(P) __sync_add_and_fetch((P), -1)
#define eal_atomic_add(P, V) __sync_add_and_fetch((P), (V))
#define eal_atomic_set_bit(P, V) __sync_or_and_fetch((P), 1 << (V))
#define eal_atomic_clear_bit(P, V) __sync_and_and_fetch((P), ~(1 << (V)))

#define eal_fetch_and_add32(ptr, value) __sync_fetch_and_add(ptr, value)
#define eal_fetch_and_add64(ptr, value) __sync_fetch_and_add(ptr, value)
#define eal_fetch_and_sub32(ptr, value) __sync_fetch_and_sub(ptr, value)
#define eal_fetch_and_sub64(ptr, value) __sync_fetch_and_sub(ptr, value)

#define eal_fetch_and_or32(ptr, value) __sync_fetch_and_or(ptr, value)
#define eal_fetch_and_or64(ptr, value) __sync_fetch_and_or(ptr, value)
#define eal_fetch_and_xor32(ptr, value) __sync_fetch_and_xor(ptr, value)
#define eal_fetch_and_xor64(ptr, value) __sync_fetch_and_xor(ptr, value)

#define eal_add_and_fetch32(ptr, value) __sync_add_and_fetch(ptr, value)
#define eal_add_and_fetch64(ptr, value) __sync_add_and_fetch(ptr, value)
#define eal_sub_and_fetch32(ptr, value) __sync_sub_and_fetch(ptr, value)
#define eal_sub_and_fetch64(ptr, value) __sync_sub_and_fetch(ptr, value)
#define eal_or_and_fetch32(ptr, value) __sync_or_and_fetch(ptr, value)
#define eal_or_and_fetch64(ptr, value) __sync_or_and_fetch(ptr, value)
#define eal_xor_and_fetch32(ptr, value) __sync_xor_and_fetch(ptr, value)
#define eal_xor_and_fetch64(ptr, value) __sync_xor_and_fetch(ptr, value)

#define eal_synchronize() __sync_synchronize()

#define eal_compare_and_swap32(ptr, oldval, newval) \
  __sync_val_compare_and_swap(ptr, oldval, newval)
#define eal_compare_and_swap64(ptr, oldval, newval) \
  __sync_val_compare_and_swap(ptr, oldval, newval)

#elif defined(_MSC_VER)
#define eal_fetch_and_add32(ptr, value) InterlockedExchangeAdd(ptr, value)
//#define eal_fetch_and_add64(ptr, value) InterlockedExchangeAdd64(ptr, value)
#define eal_fetch_and_sub32(ptr, value) InterlockedExchangeAdd(ptr, -(value))
//#define eal_fetch_and_sub64(ptr, value) InterlockedExchangeAdd64(ptr, -(value))

#define eal_compare_and_swap32(ptr, oldval, newval) \
  InterlockedCompareExchange(ptr, newval, oldval)
//#define eal_compare_and_swap64(ptr, oldval, newval) \
//  InterlockedCompareExchange64(ptr, newval, oldval)

#define eal_increment(pval) InterlockedIncrement(pval)
#define eal_decrement(pval) InterlockedDecrement(pval)
#define eal_xadd(pval, newval) InterlockedExchange(pval, newval)
#else
#warning(No atomic implementation!)
#endif

#endif /** LMICE_EAL_ATOMIC_H */

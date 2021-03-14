#ifndef _ATM_TH_GCC_SYNC_H
#define _ATM_TH_GCC_SYNC_H

#ifdef __cplusplus
extern "C"{
#endif

#include <stdint.h>

typedef intptr_t atm_t;

#define ATM_COMPILE_BARRIER_() __asm__ __volatile__("" : : : "memory")

#define atm_full_barrier() (__sync_synchronize())

#if defined(__i386) || defined(__x86_64__)
/* All loads are acquire loads and all stores are release stores.  */
#define ATM_LS_BARRIER_() ATM_COMPILE_BARRIER_()
#define TH_CACHELINE_SIZE_LOG 6

#else
#define ATM_LS_BARRIER_() atm_full_barrier()
#endif

#define TH_CACHELINE_SIZE (1 << TH_CACHELINE_SIZE_LOG)

static __inline atm_t atm_acq_load(const atm_t *p) {
  atm_t value = *p;
  ATM_LS_BARRIER_();
  return value;
}

static __inline atm_t atm_no_barrier_load(const atm_t *p) {
  atm_t value = *p;
  ATM_COMPILE_BARRIER_();
  return value;
}

static __inline void atm_rel_store(atm_t *p, atm_t value) {
  ATM_LS_BARRIER_();
  *p = value;
}

static __inline void atm_t_no_barrier_store(atm_t* p, atm_t value) {
  ATM_COMPILE_BARRIER_();
  *p = value;
}

#undef ATM_LS_BARRIER_
#undef ATM_COMPILE_BARRIER_

#define atm_full_fetch_add(p, delta) (__sync_fetch_and_add((p), (delta)))
#define atm_no_barrier_fetch_add(p, delta) \
  atm_full_fetch_add((p), (delta))

#define atm_acq_cas(p, o, n) (__sync_bool_compare_and_swap((p), (o), (n)))
#define atm_no_barrier_cas(p, o, n) atm_acq_cas((p), (o), (n))
#define atm_rel_cas(p, o, n) atm_acq_cas((p), (o), (n))
#define atm_full_cas(p, o, n) atm_acq_cas((p), (o), (n))

static __inline atm_t atm_full_xchg(atm_t *p, atm_t n) {
  atm_t cur;
  do {
    cur = atm_acq_load(p);
  } while (!atm_rel_cas(p, cur, n));
  return cur;
}

#ifdef __cplusplus
}
#endif

#endif

/* byte_order.h */
#ifndef BYTE_ORDER_H
#define BYTE_ORDER_H
#include <stdint.h>

#if _MSC_VER > 1000
#include "platform-dependent.h"
#endif

#ifdef __GLIBC__
# include <endian.h>
#endif

#if (defined(__BYTE_ORDER) && defined(__LITTLE_ENDIAN) && \
     __BYTE_ORDER == __LITTLE_ENDIAN) || \
    defined(i386) || defined(__i386__) || defined(__i486__) || \
    defined(__i586__) || defined(__i686__) || defined(__alpha__) || \
    defined(__ia64) || defined(__ia64__) || \
    defined(_M_IX86) || defined(_M_IA64) || \
    defined(_M_ALPHA) || defined(__amd64) || \
    defined(__amd64__) || defined(_M_AMD64) || \
    defined(__x86_64) || defined(__x86_64__) || defined(_M_X64) || \
    defined(vax) || defined(MIPSEL) || defined(_ARM_)
# define CPU_LITTLE_ENDIAN
#elif (defined(__BYTE_ORDER) && defined(__BIG_ENDIAN) && \
       __BYTE_ORDER == __BIG_ENDIAN) || \
    defined(__sparc) || defined(__sparc__) || defined(sparc) || \
    defined(_POWER) || defined(__powerpc__) || defined(POWERPC) || \
    defined(__ppc__) || defined(__hpux) || \
    defined(_MIPSEB) || defined(__s390__) || \
    defined(mc68000) || defined(sel)
# define CPU_BIG_ENDIAN
#else
# error "Can't detect CPU architechture"
#endif

#ifdef CPU_BIG_ENDIAN
# define IS_BIG_ENDIAN 1
# define IS_LITTLE_ENDIAN 0
#else
# define IS_BIG_ENDIAN 0
# define IS_LITTLE_ENDIAN 1
#endif

#define IS_ALIGNED_32(p) (0 == (3 & ((const char*)(p) - (const char*)0)))
#define IS_ALIGNED_64(p) (0 == (7 & ((const char*)(p) - (const char*)0)))

#ifdef __cplusplus
extern "C" {
#endif

void u32_swap_copy(void* to, const void* from, size_t length);
void u64_swap_copy(void* to, int index, const void* from, size_t length);
void u32_memswap(unsigned *p, int length_in_u32);

#if defined(__GNUC__) && (defined(__i486__) || defined(__i586__) || defined(__i686__))
static inline uint32_t bswap_32(uint32_t x) {
  __asm("bswap	%0":
      "=r" (x)     :
      "0" (x));
  return x;
}
#else
static inline uint32_t bswap_32(uint32_t x) {
    x= ((x<<8)&0xFF00FF00) | ((x>>8)&0x00FF00FF);
    return (x>>16) | (x<<16);
}
#endif

static inline uint64_t bswap_64(uint64_t x) {
    union {
        uint64_t ll;
        uint32_t l[2];
    } w, r;
    w.ll = x;
    r.l[0] = bswap_32(w.l[1]);
    r.l[1] = bswap_32(w.l[0]);
    return r.ll;
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#ifdef CPU_BIG_ENDIAN
# define be2me_32(x) (x)
# define be2me_64(x) (x)
# define le2me_32(x) bswap_32(x)
# define le2me_64(x) bswap_64(x)

# define be32_copy memcpy
# define le32_copy u32_swap_copy
# define be64_copy(to, index, from, length) memcpy(to + index, from, length)
//# define le64_copy(to, index, from, length) u64_swap_copy(to, index, from, length)
#else /* CPU_BIG_ENDIAN */
# define be2me_32(x) bswap_32(x)
# define be2me_64(x) bswap_64(x)
# define le2me_32(x) (x)
# define le2me_64(x) (x)

# define be32_copy u32_swap_copy
# define le32_copy memcpy
# define be64_copy(to, index, from, length) u64_swap_copy(to, index, from, length)
# define le64_copy(to, index, from, length) memcpy(to + index, from, length)
#endif /* CPU_BIG_ENDIAN */

#endif /* BYTE_ORDER_H */

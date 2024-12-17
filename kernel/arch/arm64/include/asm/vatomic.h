#ifndef VATOMIC_H
#define VATOMIC_H
#include <linux/types.h>

#define vatomic_t vatomic32_t
#define vuint8_t uint8_t
#define vuint16_t uint16_t
#define vuint32_t uint32_t
#define vuint64_t uint64_t

struct vatomic32_s {
 uint32_t __v;
} __attribute__((aligned(4)));
typedef struct vatomic32_s vatomic32_t;

struct vatomic64_s {
 uint64_t __v;
} __attribute__((aligned(8)));
typedef struct vatomic64_s vatomic64_t;

typedef unsigned long vuintptr_t;

#define vatomic_read_rlx vatomic32_read_rlx
#define vatomic_read_acq vatomic32_read_acq

#define vatomic_write_rlx vatomic32_write_rlx
#define vatomic_write_rel vatomic32_write_rel

#define vatomic_inc_rel vatomic32_inc_rel
#define vatomic_cmpxchg_rlx vatomic32_cmpxchg_rlx

static inline uint32_t vatomic32_read_rlx(vatomic32_t *a)
{
    uint32_t val;
    __asm__ volatile( "ld""r"" %""w""[v], %[a]"
    : [v] "=&r"(val)
    : [a] "Q"(a->__v)
    : "memory" );
    return val;
}
static inline uint64_t vatomic64_read_rlx(vatomic64_t *a)
{
    uint64_t val;
    __asm__ volatile( "ld""r"" %""x""[v], %[a]" : [v] "=&r"(val) : [a] "Q"(a->__v) : "memory" );
    return val;
}

static inline void vatomic64_write(vatomic64_t *a, uint64_t v)
{
    __asm__ volatile( "st""l""r"" %""x""[v], %[a]"
    :
    : [v] "r"(v), [a] "Q"(a->__v)
    : "memory" );
}

static inline void vatomic32_write_rlx(vatomic32_t *a, uint32_t v)
{
    __asm__ volatile( "st""r"" %""w""[v], %[a]"
    :
    : [v] "r"(v), [a] "Q"(a->__v)
    : "memory" );
}
static inline void vatomic64_write_rlx(vatomic64_t *a, uint64_t v)
{
    __asm__ volatile( "st""r"" %""x""[v], %[a]" : : [v] "r"(v), [a] "Q"(a->__v) : "memory" );
}

static inline void vatomic32_write_rel(vatomic32_t *a, uint32_t v)
{
    __asm__ volatile( "st""l""r"" %""w""[v], %[a]"
    :
    : [v] "r"(v), [a] "Q"(a->__v)
    : "memory" );
}
static inline void vatomic64_write_rel(vatomic64_t *a, uint64_t v)
{
    __asm__ volatile( "st""l""r"" %""x""[v], %[a]" : : [v] "r"(v), [a] "Q"(a->__v) : "memory" );
}

static inline uint32_t vatomic32_add_rel(vatomic32_t *a, uint32_t v)
{
    uint32_t oldv;
    uint32_t newv;
    uint32_t tmp;
    __asm__ volatile( "   prfm pstl1strm, %[a]\n"
"1:""ld""xr""	 	%""w""[oldv], %[a]\n"
"  ""add""			%""w""[newv], %""w""[oldv], %""w""[v]\n"
"  ""st""l""xr""	 	%w[tmp], %""w""[newv], %[a]\n"
"   cbnz 		%w[tmp], 1b\n"
: [oldv] "=&r"(oldv), [newv] "=&r"(newv), [tmp] "=&r"(tmp)
: [v] "r"(v), [a] "Q"(a->__v)
: "memory" );
    return newv;
    }

static inline uint32_t vatomic32_inc_rel(vatomic32_t *a)
{
    return vatomic32_add_rel(a, 1U);
}

static inline uint32_t vatomic32_cmpxchg_rlx(vatomic32_t *a, uint32_t e, uint32_t v)
{
    uint32_t old;
    uint32_t tmp;
    __asm__ volatile( "   prfm pstl1strm, %[a]\n"
"1:""ld""xr""	%""w""[oldv], %[a]\n"
"   cmp		%""w""[oldv], %""w""[expv]\n"
"   b.ne	2f\n"
"  ""st""xr""	%w[tmp], %""w""[newv], %[a]\n"
"   cbnz 	%w[tmp], 1b\n"
"2:\n"
    : [oldv] "=&r"(old), [tmp] "=&r"(tmp)
    : [newv] "r"(v), [expv] "r"(e), [a] "Q"(a->__v)
    : "memory" );
    return old;
}
static inline uint64_t vatomic64_cmpxchg_rlx(vatomic64_t *a, uint64_t e, uint64_t v)
{
    uint64_t old;
    uint32_t tmp;
    __asm__ volatile( "   prfm pstl1strm, %[a]\n"
"1:""ld""xr""	%""x""[oldv], %[a]\n"
"   cmp		%""x""[oldv], %""x""[expv]\n"
"   b.ne	2f\n"
"  ""st""xr""	%w[tmp], %""x""[newv], %[a]\n"
"   cbnz 	%w[tmp], 1b\n"
"2:\n"
    : [oldv] "=&r"(old), [tmp] "=&r"(tmp)
    : [newv] "r"(v), [expv] "r"(e), [a] "Q"(a->__v)
    : "memory" );
    return old;
}

static inline uint32_t vatomic32_read_acq(vatomic32_t *a)
{
    uint32_t val;
    __asm__ volatile( "ld""a""r"" %""w""[v], %[a]"
    : [v] "=&r"(val)
    : [a] "Q"(a->__v)
    : "memory" );
    return val;
}
static inline uint64_t vatomic64_read_acq(vatomic64_t *a)
{
    uint64_t val;
    __asm__ volatile( "ld""a""r"" %""x""[v], %[a]"
    : [v] "=&r"(val)
    : [a] "Q"(a->__v)
    : "memory" );
    return val;
}

static inline void vatomic64_init(vatomic64_t *a, uint64_t v)
{
    vatomic64_write(a, v);
}

static inline void vatomic_fence(void)
{
    __asm__ volatile( "dmb ish" ::: "memory" );
}
static inline void vatomic_fence_acq(void)
{
    __asm__ volatile( "dmb ishld" ::: "memory" );
}

#endif /* VSYNC_ATOMIC_H */

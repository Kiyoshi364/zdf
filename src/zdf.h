#ifndef _ZDF_H_
#define _ZDF_H_

#ifndef ZDF_INT
#include <stdint.h>
#define ZDF_INT int32_t

#ifndef ZDF_INT_BITS
#define ZDF_INT_BITS 32
#else
#error "ZDF_INT is defined but ZDF_INT_BITS is not"
#endif // ZDF_INT_BITS

#ifndef ZDF_LONG
#define ZDF_LONG int64_t
#else
#error "ZDF_INT is defined but ZDF_LONG is not"
#endif // ZDF_LONG

#endif // ZDF_INT

#ifndef ZDF_FUNC
#define ZDF_FUNC(name) zdf_ ## name
#endif // ZDF_FUNC

ZDF_INT ZDF_FUNC(lisqrt)(ZDF_LONG n);
ZDF_INT ZDF_FUNC(circle)(ZDF_INT cx, ZDF_INT cy, ZDF_INT r, ZDF_INT px, ZDF_INT py);

#endif // _ZDF_H_

#ifdef ZDF_IMPLEMENTATION

ZDF_INT ZDF_FUNC(lisqrt)(ZDF_LONG n) {
    ZDF_LONG acc = 0;
    ZDF_INT res = 0;
    for (ZDF_INT i = (1 << (((ZDF_INT_BITS - 1)/2) + 1 - 1)); 0 < i; i >>= 1) {
        const ZDF_LONG i_ = (ZDF_LONG) i;
        const ZDF_LONG new_acc = acc + 2*((ZDF_LONG) res)*i_ + i_*i_;
        if (new_acc <= n) {
            acc = new_acc;
            res += i;
        } else {
            // Nothing
        }
    }
    return res;
}

ZDF_INT ZDF_FUNC(circle)(ZDF_INT cx, ZDF_INT cy, ZDF_INT r, ZDF_INT px, ZDF_INT py) {
    const ZDF_LONG dx = (ZDF_LONG) (px - cx);
    const ZDF_LONG dy = (ZDF_LONG) (py - cy);
    return ZDF_FUNC(lisqrt)(dx*dx + dy*dy) - ((ZDF_LONG) r);
}

#endif // ZDF_IMPLEMENTATION

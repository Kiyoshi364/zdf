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

#ifndef ZDF_TYPE
#define ZDF_TYPE(name) Zdf ## name
#endif // ZDF_TYPE

#ifndef ZDF_FUNC
#define ZDF_FUNC(name) zdf_ ## name
#endif // ZDF_FUNC

ZDF_LONG ZDF_FUNC(mult)(ZDF_INT a, ZDF_INT b);
ZDF_INT ZDF_FUNC(ilen)(ZDF_INT x, ZDF_INT y);

ZDF_INT ZDF_FUNC(lisqrt)(ZDF_LONG n);

typedef struct {
    ZDF_INT cx;
    ZDF_INT cy;
    ZDF_INT r;
} ZDF_TYPE(Circle);

ZDF_INT ZDF_FUNC(circle)(ZDF_TYPE(Circle) circle, ZDF_INT px, ZDF_INT py);

#endif // _ZDF_H_

#ifdef ZDF_IMPLEMENTATION

ZDF_LONG ZDF_FUNC(mult)(ZDF_INT a, ZDF_INT b) {
    return ((ZDF_LONG) a) * ((ZDF_LONG) b);
}

ZDF_INT ZDF_FUNC(lisqrt)(ZDF_LONG n) {
    ZDF_LONG acc = 0;
    ZDF_INT res = 0;
    for (ZDF_INT i = (1 << (((ZDF_INT_BITS - 1)/2) + 1 - 1)); 0 < i; i >>= 1) {
        const ZDF_LONG new_acc = acc + 2*ZDF_FUNC(mult)(res, i) + ZDF_FUNC(mult)(i, i);
        if (new_acc <= n) {
            acc = new_acc;
            res += i;
        } else {
            // Nothing
        }
    }
    return res;
}

ZDF_INT ZDF_FUNC(ilen)(ZDF_INT x, ZDF_INT y) {
    const ZDF_LONG xx = ZDF_FUNC(mult)(x, x);
    const ZDF_LONG yy = ZDF_FUNC(mult)(y, y);
    return ZDF_FUNC(lisqrt)(xx + yy);
}

ZDF_INT ZDF_FUNC(circle)(ZDF_TYPE(Circle) circle, ZDF_INT px, ZDF_INT py) {
    const ZDF_INT dx = px - circle.cx;
    const ZDF_INT dy = py - circle.cy;
    return ZDF_FUNC(ilen)(dx, dy) - circle.r;
}

#endif // ZDF_IMPLEMENTATION

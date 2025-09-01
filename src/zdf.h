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

ZDF_LONG ZDF_FUNC(imul)(ZDF_INT a, ZDF_INT b);
ZDF_LONG ZDF_FUNC(idot)(ZDF_INT x1, ZDF_INT y1, ZDF_INT x2, ZDF_INT y2);
ZDF_LONG ZDF_FUNC(idot2)(ZDF_INT x, ZDF_INT y);

ZDF_INT ZDF_FUNC(ilen)(ZDF_INT x, ZDF_INT y);

ZDF_INT ZDF_FUNC(lidiv)(ZDF_LONG a, ZDF_INT b);
ZDF_INT ZDF_FUNC(lisqrt)(ZDF_LONG n);

typedef struct {
    ZDF_INT cx;
    ZDF_INT cy;
    ZDF_INT r;
} ZDF_TYPE(Circle);

ZDF_INT ZDF_FUNC(circle)(ZDF_TYPE(Circle) circle, ZDF_INT px, ZDF_INT py);

typedef struct {
    ZDF_INT cx;
    ZDF_INT cy;
    ZDF_INT nx;
    ZDF_INT ny;
} ZDF_TYPE(Line);

ZDF_INT ZDF_FUNC(line)(ZDF_TYPE(Line) line, ZDF_INT px, ZDF_INT py);

#endif // _ZDF_H_

#ifdef ZDF_IMPLEMENTATION

ZDF_LONG ZDF_FUNC(imul)(ZDF_INT a, ZDF_INT b) {
    return ((ZDF_LONG) a) * ((ZDF_LONG) b);
}

ZDF_LONG ZDF_FUNC(idot)(ZDF_INT x1, ZDF_INT y1, ZDF_INT x2, ZDF_INT y2) {
    const ZDF_LONG xx = ZDF_FUNC(imul)(x1, x2);
    const ZDF_LONG yy = ZDF_FUNC(imul)(y1, y2);
    return xx + yy;
}

ZDF_LONG ZDF_FUNC(idot2)(ZDF_INT x, ZDF_INT y) {
    return ZDF_FUNC(idot)(x, y, x, y);
}

ZDF_INT ZDF_FUNC(lidiv)(ZDF_LONG a, ZDF_INT b) {
    const ZDF_LONG res = a / ((ZDF_LONG) b);
    const ZDF_LONG mask = (1ULL << ZDF_INT_BITS) - 1;
    return (ZDF_INT) (res & mask);
}

ZDF_INT ZDF_FUNC(lisqrt)(ZDF_LONG n) {
    ZDF_LONG acc = 0;
    ZDF_INT res = 0;
    for (ZDF_INT i = (1 << (((ZDF_INT_BITS - 1)/2) + 1 - 1)); 0 < i; i >>= 1) {
        const ZDF_LONG new_acc = acc + ZDF_FUNC(idot)(2*res, i, i, i);
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
    return ZDF_FUNC(lisqrt)(ZDF_FUNC(idot2)(x, y));
}

ZDF_INT ZDF_FUNC(circle)(ZDF_TYPE(Circle) circle, ZDF_INT px, ZDF_INT py) {
    const ZDF_INT dx = px - circle.cx;
    const ZDF_INT dy = py - circle.cy;
    return ZDF_FUNC(ilen)(dx, dy) - circle.r;
}

ZDF_INT ZDF_FUNC(line)(ZDF_TYPE(Line) line, ZDF_INT px, ZDF_INT py) {
    const ZDF_INT dx = px - line.cx;
    const ZDF_INT dy = py - line.cy;
    const ZDF_LONG dot = ZDF_FUNC(idot)(dx, dy, line.nx, line.ny);
    const ZDF_INT len = ZDF_FUNC(ilen)(line.nx, line.ny);
    return ZDF_FUNC(lidiv)(dot, len);
}

#endif // ZDF_IMPLEMENTATION

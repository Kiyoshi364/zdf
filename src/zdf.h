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

typedef struct {
    ZDF_INT x;
    ZDF_INT y;
} ZDF_TYPE(Vec2);

ZDF_LONG ZDF_FUNC(imul)(ZDF_INT a, ZDF_INT b);
ZDF_LONG ZDF_FUNC(idot)(ZDF_INT x1, ZDF_INT y1, ZDF_INT x2, ZDF_INT y2);
ZDF_LONG ZDF_FUNC(idot2)(ZDF_INT x, ZDF_INT y);
ZDF_INT ZDF_FUNC(ilen)(ZDF_INT x, ZDF_INT y);

ZDF_TYPE(Vec2) ZDF_FUNC(ivscale)(ZDF_TYPE(Vec2) v, int mul, int div);
ZDF_TYPE(Vec2) ZDF_FUNC(ivadd)(ZDF_TYPE(Vec2) v1, ZDF_TYPE(Vec2) v2);
ZDF_TYPE(Vec2) ZDF_FUNC(ivsub)(ZDF_TYPE(Vec2) v1, ZDF_TYPE(Vec2) v2);
ZDF_LONG ZDF_FUNC(ivdot)(ZDF_TYPE(Vec2) v1, ZDF_TYPE(Vec2) v2);
ZDF_LONG ZDF_FUNC(ivdot2)(ZDF_TYPE(Vec2) v);
ZDF_INT ZDF_FUNC(ivlen)(ZDF_TYPE(Vec2) v);

ZDF_INT ZDF_FUNC(lidiv)(ZDF_LONG a, ZDF_INT b);
ZDF_INT ZDF_FUNC(lisqrt)(ZDF_LONG n);

typedef struct {
    ZDF_TYPE(Vec2) c;
    ZDF_INT r;
} ZDF_TYPE(Circle);

ZDF_INT ZDF_FUNC(circle)(ZDF_TYPE(Circle) circle, ZDF_TYPE(Vec2) p);
ZDF_TYPE(Vec2) ZDF_FUNC(circle_grad)(ZDF_TYPE(Circle) circle, ZDF_TYPE(Vec2) p, ZDF_INT one);

typedef struct {
    ZDF_TYPE(Vec2) c;
    ZDF_TYPE(Vec2) n;
} ZDF_TYPE(Line);

ZDF_INT ZDF_FUNC(line)(ZDF_TYPE(Line) line, ZDF_TYPE(Vec2) p);
ZDF_TYPE(Vec2) ZDF_FUNC(line_grad)(ZDF_TYPE(Line) line, ZDF_TYPE(Vec2) p, ZDF_INT one);

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

ZDF_INT ZDF_FUNC(ilen)(ZDF_INT x, ZDF_INT y) {
    return ZDF_FUNC(lisqrt)(ZDF_FUNC(idot2)(x, y));
}

ZDF_TYPE(Vec2) ZDF_FUNC(ivscale)(ZDF_TYPE(Vec2) v, int mul, int div) {
    return (ZDF_TYPE(Vec2)){
        .x = v.x * mul / div,
        .y = v.y * mul / div,
    };
}

ZDF_TYPE(Vec2) ZDF_FUNC(ivadd)(ZDF_TYPE(Vec2) v1, ZDF_TYPE(Vec2) v2) {
    return (ZDF_TYPE(Vec2)){
        .x = v1.x + v2.x,
        .y = v1.y + v2.y,
    };
}

ZDF_TYPE(Vec2) ZDF_FUNC(ivsub)(ZDF_TYPE(Vec2) v1, ZDF_TYPE(Vec2) v2) {
    return (ZDF_TYPE(Vec2)){
        .x = v1.x - v2.x,
        .y = v1.y - v2.y,
    };
}

ZDF_LONG ZDF_FUNC(ivdot)(ZDF_TYPE(Vec2) v1, ZDF_TYPE(Vec2) v2) {
    return ZDF_FUNC(idot)(v1.x, v1.y, v2.x, v2.y);
}

ZDF_LONG ZDF_FUNC(ivdot2)(ZDF_TYPE(Vec2) v) {
    return ZDF_FUNC(ivdot)(v, v);
}

ZDF_INT ZDF_FUNC(ivlen)(ZDF_TYPE(Vec2) v) {
    return ZDF_FUNC(ilen)(v.x, v.y);
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

ZDF_INT ZDF_FUNC(circle)(ZDF_TYPE(Circle) circle, ZDF_TYPE(Vec2) p) {
    const ZDF_TYPE(Vec2) d = ZDF_FUNC(ivsub)(p, circle.c);
    return ZDF_FUNC(ivlen)(d) - circle.r;
}

ZDF_TYPE(Vec2) ZDF_FUNC(circle_grad)(ZDF_TYPE(Circle) circle, ZDF_TYPE(Vec2) p, ZDF_INT one) {
    const ZDF_TYPE(Vec2) n = (ZDF_TYPE(Vec2)){
        .x = p.x - circle.c.x,
        .y = p.y - circle.c.y,
    };
    const ZDF_INT len = ZDF_FUNC(ivlen)(n);
    const ZDF_LONG nx_ = ZDF_FUNC(imul)(n.x, one);
    const ZDF_LONG ny_ = ZDF_FUNC(imul)(n.y, one);
    return (ZDF_TYPE(Vec2)){
        .x = ZDF_FUNC(lidiv)(nx_, len),
        .y = ZDF_FUNC(lidiv)(ny_, len),
    };
}

ZDF_INT ZDF_FUNC(line)(ZDF_TYPE(Line) line, ZDF_TYPE(Vec2) p) {
    const ZDF_TYPE(Vec2) d = ZDF_FUNC(ivsub)(p, line.c);
    const ZDF_LONG dot = ZDF_FUNC(ivdot)(d, line.n);
    const ZDF_INT len = ZDF_FUNC(ivlen)(line.n);
    return ZDF_FUNC(lidiv)(dot, len);
}

ZDF_TYPE(Vec2) ZDF_FUNC(line_grad)(ZDF_TYPE(Line) line, ZDF_TYPE(Vec2) p, ZDF_INT one) {
    if (ZDF_FUNC(ivdot)(p, line.n) < 0) {
        one *= -1;
    }
    const ZDF_INT len = ZDF_FUNC(ivlen)(line.n);
    const ZDF_LONG nx_ = ZDF_FUNC(imul)(line.n.x, one);
    const ZDF_LONG ny_ = ZDF_FUNC(imul)(line.n.y, one);
    return (ZDF_TYPE(Vec2)){
        .x = ZDF_FUNC(lidiv)(nx_, len),
        .y = ZDF_FUNC(lidiv)(ny_, len),
    };
}

#endif // ZDF_IMPLEMENTATION

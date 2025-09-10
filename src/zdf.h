#ifndef _ZDF_H_
#define _ZDF_H_

#ifdef ZDF_INT

#ifndef ZDF_LONG
#error "ZDF_LONG is not defined but ZDF_INT is"
#define ZDF_LONG long long int
#endif // ZDF_LONG

#else // ZDF_INT

#include <stdint.h>
#define ZDF_INT int32_t

#ifdef ZDF_LONG
#error "ZDF_LONG is defined but ZDF_INT is not"
#define ZDF_LONG int64_t
#else // ZDF_LONG
#define ZDF_LONG int64_t
#endif // ZDF_LONG

#endif // ZDF_INT

#ifndef ZDF_INT_BITS
#define ZDF_INT_BITS (sizeof(ZDF_INT) << 3)
#endif // ZDF_INT_BITS

#ifndef ZDF_LONG_BITS
#define ZDF_LONG_BITS (sizeof(ZDF_LONG) << 3)
#endif // ZDF_LONG_BITS

#ifndef ZDF_TYPE
#define ZDF_TYPE(name) Zdf ## name
#endif // ZDF_TYPE

#ifndef ZDF_FUNC
#define ZDF_FUNC(name) zdf_ ## name
#endif // ZDF_FUNC

ZDF_INT ZDF_FUNC(iargmin)(ZDF_INT a, ZDF_INT b, ZDF_INT xa, ZDF_INT xb);
ZDF_INT ZDF_FUNC(iargmax)(ZDF_INT a, ZDF_INT b, ZDF_INT xa, ZDF_INT xb);
ZDF_INT ZDF_FUNC(imin)(ZDF_INT a, ZDF_INT b);
ZDF_INT ZDF_FUNC(imax)(ZDF_INT a, ZDF_INT b);
ZDF_INT ZDF_FUNC(iabs)(ZDF_INT a);
ZDF_INT ZDF_FUNC(iclamp)(ZDF_INT min, ZDF_INT max, ZDF_INT x);

ZDF_LONG ZDF_FUNC(imul)(ZDF_INT a, ZDF_INT b);
ZDF_LONG ZDF_FUNC(idot)(ZDF_INT x1, ZDF_INT y1, ZDF_INT x2, ZDF_INT y2);
ZDF_LONG ZDF_FUNC(idot2)(ZDF_INT x, ZDF_INT y);
ZDF_INT ZDF_FUNC(ilen)(ZDF_INT x, ZDF_INT y);

typedef struct {
    ZDF_INT x;
    ZDF_INT y;
} ZDF_TYPE(Vec2);

ZDF_TYPE(Vec2) ZDF_FUNC(ivscale)(ZDF_TYPE(Vec2) v, int mul, int div);
ZDF_TYPE(Vec2) ZDF_FUNC(ivadd)(ZDF_TYPE(Vec2) v1, ZDF_TYPE(Vec2) v2);
ZDF_TYPE(Vec2) ZDF_FUNC(ivsub)(ZDF_TYPE(Vec2) v1, ZDF_TYPE(Vec2) v2);
ZDF_LONG ZDF_FUNC(ivdot)(ZDF_TYPE(Vec2) v1, ZDF_TYPE(Vec2) v2);
ZDF_LONG ZDF_FUNC(ivdot2)(ZDF_TYPE(Vec2) v);
ZDF_INT ZDF_FUNC(ivlen)(ZDF_TYPE(Vec2) v);
ZDF_TYPE(Vec2) ZDF_FUNC(ivnormal)(ZDF_TYPE(Vec2) v, ZDF_INT one);

ZDF_INT ZDF_FUNC(lidiv)(ZDF_LONG a, ZDF_INT b);
ZDF_INT ZDF_FUNC(lisqrt)(ZDF_LONG n);

// REF: https://iquilezles.org/articles/distfunctions2d/
// REF: https://iquilezles.org/articles/distgradfunctions2d/

typedef struct {
    ZDF_TYPE(Vec2) c;
    ZDF_INT r;
} ZDF_TYPE(Circle);

ZDF_INT ZDF_FUNC(circle)(ZDF_TYPE(Circle) circle, ZDF_TYPE(Vec2) p);
ZDF_TYPE(Vec2) ZDF_FUNC(circle_grad)(ZDF_TYPE(Circle) circle, ZDF_TYPE(Vec2) p, ZDF_INT one);

typedef struct {
    ZDF_TYPE(Vec2) n;
    ZDF_INT off;
} ZDF_TYPE(Line);

ZDF_INT ZDF_FUNC(line)(ZDF_TYPE(Line) line, ZDF_TYPE(Vec2) p, ZDF_INT one);
ZDF_TYPE(Vec2) ZDF_FUNC(line_grad)(ZDF_TYPE(Line) line, ZDF_TYPE(Vec2) p);

#endif // _ZDF_H_

#ifdef ZDF_IMPLEMENTATION

#ifndef ZDF_ASSERT
#define ZDF_ASSERT ZDF_FUNC(__assert)
void ZDF_FUNC(__assert)(...) {}
#endif // ZDF_ASSERT

ZDF_INT ZDF_FUNC(iargmin)(ZDF_INT a, ZDF_INT b, ZDF_INT xa, ZDF_INT xb) {
    return (a < b) ? xa : xb;
}

ZDF_INT ZDF_FUNC(iargmax)(ZDF_INT a, ZDF_INT b, ZDF_INT xa, ZDF_INT xb) {
    return ZDF_FUNC(iargmin)(a, b, xb, xa);
}

ZDF_INT ZDF_FUNC(imin)(ZDF_INT a, ZDF_INT b) {
    return ZDF_FUNC(iargmin)(a, b, a, b);
}

ZDF_INT ZDF_FUNC(imax)(ZDF_INT a, ZDF_INT b) {
    return ZDF_FUNC(iargmax)(a, b, a, b);
}

ZDF_INT ZDF_FUNC(iabs)(ZDF_INT a) {
    return ZDF_FUNC(imax)(a, -a);
}

ZDF_INT ZDF_FUNC(iclamp)(ZDF_INT min, ZDF_INT max, ZDF_INT x) {
    return ZDF_FUNC(imax)(min, ZDF_FUNC(imin)(x, max));
}


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

ZDF_TYPE(Vec2) ZDF_FUNC(ivnormal)(ZDF_TYPE(Vec2) v, ZDF_INT one) {
    const ZDF_INT len = ZDF_FUNC(ivlen)(v);
    const ZDF_LONG x_ = ZDF_FUNC(imul)(v.x, one);
    const ZDF_LONG y_ = ZDF_FUNC(imul)(v.y, one);
    return (ZDF_TYPE(Vec2)){
        .x = ZDF_FUNC(lidiv)(x_, len),
        .y = ZDF_FUNC(lidiv)(y_, len),
    };
}

ZDF_INT ZDF_FUNC(lidiv)(ZDF_LONG a, ZDF_INT b) {
    const ZDF_LONG res = a / ((ZDF_LONG) b);
    const ZDF_LONG mask = (1ULL << ZDF_INT_BITS) - 1;
    return (ZDF_INT) (res & mask);
}

// REF: https://wikipedia.org/wiki/Square_root_algorithms#Digit-by-digit_calculation
ZDF_INT ZDF_FUNC(lisqrt)(ZDF_LONG n) {
    // NOTE: code inside ZDF_NO_TRUST_LISQRT
    // is a proof sketch for correctness
    ZDF_LONG remainder = n;
    const ZDF_INT initial_one2_power = (((ZDF_LONG_BITS - 1) >> 2) << 2);
    ZDF_LONG one2 = ((ZDF_LONG) 1) << initial_one2_power;
    ZDF_LONG two_res_one = 0;

    // NOTE: one2 == pow(4, initial_one2_power >> 1)

#ifdef ZDF_NOTRUST_LISQRT
    ZDF_INT one = ((ZDF_INT) 1) << (initial_one2_power >> 1);
    ZDF_INT res = 0;
    ZDF_ASSERT(one2 == ((ZDF_LONG) one)*((ZDF_LONG) one));
    // NOTE: one2 == pow(2, initial_one2_power >> 1)
#endif // ZDF_NOTRUST_LISQRT

    while (remainder < one2) {
        one2 >>= 2;
#ifdef ZDF_NOTRUST_LISQRT
        one >>= 1;
        ZDF_ASSERT(one2 == ((ZDF_LONG) one)*((ZDF_LONG) one));
#endif // ZDF_NOTRUST_LISQRT
    }

    while (0 < one2) {
        if (one2 + two_res_one <= remainder) {
            remainder -= (one2 + two_res_one);
            two_res_one = (two_res_one >> 1) + one2;

#ifdef ZDF_NOTRUST_LISQRT
            res += one;
#endif // ZDF_NOTRUST_LISQRT
        } else {
            two_res_one = (two_res_one >> 1);
        }
        one2 >>= 2;
#ifdef ZDF_NOTRUST_LISQRT
        one >>= 1;
        ZDF_ASSERT(one2 == ((ZDF_LONG) one)*((ZDF_LONG) one));
        ZDF_ASSERT(n == res*res + remainder);
        ZDF_ASSERT(remainder < 2*(((ZDF_LONG) res)*(2*((ZDF_LONG) one)+1)) + (2*((ZDF_LONG) one)+1)*(2*((ZDF_LONG) one)+1));
        ZDF_ASSERT(
            two_res_one == 2*(((ZDF_LONG) res)*((ZDF_LONG) one))
            || two_res_one == res
        );
#endif // ZDF_NOTRUST_LISQRT
    }

#ifdef ZDF_NOTRUST_LISQRT
    ZDF_ASSERT(two_res_one == res);
#endif // ZDF_NOTRUST_LISQRT
    return (ZDF_INT) two_res_one;
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

ZDF_INT ZDF_FUNC(line)(ZDF_TYPE(Line) line, ZDF_TYPE(Vec2) p, ZDF_INT one) {
    const ZDF_LONG dot = ZDF_FUNC(ivdot)(p, line.n);
    return ZDF_FUNC(lidiv)(dot, one) + line.off;
}

ZDF_TYPE(Vec2) ZDF_FUNC(line_grad)(ZDF_TYPE(Line) line, ZDF_TYPE(Vec2) p) {
    (void) p;
    return line.n;
}

#endif // ZDF_IMPLEMENTATION

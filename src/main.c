#include <assert.h>
#include <stdint.h>

int32_t rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    const uint32_t r_ = (uint32_t) r;
    const uint32_t g_ = (uint32_t) g;
    const uint32_t b_ = (uint32_t) b;
    const uint32_t a_ = (uint32_t) a;
    return (a_ << (8*3)) | (b_ << (8*2)) | (g_ << (8*1)) | (r_ << (8*0));
}

int32_t rgb(uint8_t r, uint8_t g, uint8_t b) {
    return rgba(r, g, b, 0xFF);
}

#include "zdf.h"
#include <stdio.h>
#include <stdint.h>

#define ARRLEN(x) ((sizeof(x))/(sizeof(x[0])))

void gen(FILE *out, const uint32_t *canvas, uint32_t w, uint32_t h, uint32_t stride, uint32_t uw, uint32_t uh);

#define FIXONE 0x10
#define WIDTH 32
#define HEIGHT 24
#define UPSCALE 0x10

void distmap_to_canvas(
    const int32_t *distmap, uint32_t w, uint32_t h, uint32_t stride,
    uint32_t *canvas,
    uint32_t border
) {
    uint32_t max_neg = 0;
    uint32_t max_pos = 0;
    for (uint32_t j = 0; j < h; j += 1) {
        for (uint32_t i = 0; i < w; i += 1) {
            const int32_t d = distmap[j*stride + i];
            if (d <= 0 && max_neg < ((uint32_t) -d)) {
                max_neg = (uint32_t) -d;
            }
            if (0 <= d && max_pos < ((uint32_t) d)) {
                max_pos = (uint32_t) d;
            }
        }
    }

    for (uint32_t j = 0; j < h; j += 1) {
        for (uint32_t i = 0; i < w; i += 1) {
            const int32_t idx = j*stride + i;
            const int32_t d = distmap[idx];
            const uint32_t ud = (uint32_t) ((d < 0) ? -d : d);

            uint8_t r = 0;
            uint8_t g = 0;
            uint8_t b = 0;

            if (d < 0) {
                b = ud * 0xFF / max_neg;
            } else {
                r = ud * 0xFF / max_pos;
            }
            if (ud < border) {
                g = (border - ud) * 0xFF / border;
            }
            canvas[idx] = rgb(r, g, b);
        }
    }
}

typedef struct {
    ZdfVec2 off;
    int32_t mul;
    int32_t div;
} Camera2D;

static
ZdfVec2 camera_to_world(const Camera2D camera, uint32_t i, uint32_t j) {
    const ZdfVec2 v = (ZdfVec2){
        .x = ((int32_t) i)*FIXONE,
        .y = ((int32_t) j)*FIXONE,
    };
    return zdf_ivscale(zdf_ivsub(v, camera.off), camera.mul, camera.div);
}

int32_t sdf_dist(const ZdfCircle circles[], uint32_t circle_len, const ZdfLine lines[], uint32_t line_len, ZdfVec2 p);

int main(void) {
    const uint32_t w = WIDTH;
    const uint32_t h = HEIGHT;
    int32_t distmap[WIDTH*HEIGHT];
    uint32_t canvas[WIDTH*HEIGHT];

    const Camera2D camera = (Camera2D){
        .off = (ZdfVec2){
            .x = (w - 1)*FIXONE/2,
            .y = (h - 1)*FIXONE/2,
        },
        .mul = 3,
        .div = 4,
    };
    const uint32_t border = FIXONE*2/8;

    const ZdfCircle circles[] = {
        (ZdfCircle){
            .c = (ZdfVec2){
                .x = - 9*FIXONE/2,
                .y = - 0*FIXONE,
            },
            .r = 5*FIXONE,
        },
        (ZdfCircle){
            .c = (ZdfVec2){
                .x = 9*FIXONE/2,
                .y = 0*FIXONE,
            },
            .r = 5*FIXONE,
        },
    };
    const ZdfLine lines[] = {
        (ZdfLine){
            .c = (ZdfVec2){
                .x = 17*FIXONE/2,
                .y = 0*FIXONE,
            },
            .n = (ZdfVec2){
                .x = - 1*FIXONE,
                .y = 1*FIXONE,
            },
        },
        (ZdfLine){
            .c = (ZdfVec2){
                .x = 17*FIXONE/2,
                .y = 0*FIXONE,
            },
            .n = (ZdfVec2){
                .x = - 1*FIXONE,
                .y = - 1*FIXONE,
            },
        },
    };

    for (uint32_t j = 0; j < h; j += 1) {
        for (uint32_t i = 0; i < w; i += 1) {
            const ZdfVec2 p = camera_to_world(camera, i, j);
            int32_t dist = sdf_dist(circles, ARRLEN(circles), lines, ARRLEN(lines), p);
            distmap[j*w + i] = dist;
        }
    }

    distmap_to_canvas(
        distmap, w, h, w,
        canvas,
        border
    );

    FILE *out = fopen("img.ppm", "w");
    gen(out, canvas, w, h, w, UPSCALE, UPSCALE);
    fclose(out);

    return 0;
}

int32_t sdf_dist(const ZdfCircle circles[], uint32_t circle_len, const ZdfLine lines[], uint32_t line_len, ZdfVec2 p) {
    assert(0 < circle_len);
    int32_t dist = zdf_circle(circles[0], p);

    for (uint32_t k = 1; k < circle_len; k += 1) {
        const int32_t d = zdf_circle(circles[k], p);
        dist = (d < dist) ? d : dist;
    }
    for (uint32_t k = 0; k < line_len; k += 1) {
        const int32_t d = zdf_line(lines[k], p);
        dist = (d < dist) ? d : dist;
    }
    return dist;
}

#define ZDF_IMPLEMENTATION
#include "zdf.h"

// PBM serializer (to P6)
void gen(FILE *out, const uint32_t *canvas, uint32_t w, uint32_t h, uint32_t stride, uint32_t uw, uint32_t uh) {
    fprintf(out, "P6 %d %d 255\n", w*uw, h*uh);
    for (uint32_t j = 0; j < h; j += 1) {
        for (uint32_t jj = 0; jj < uh; jj += 1) {
            for (uint32_t i = 0; i < w; i += 1) {
                const uint32_t p = canvas[j*stride + i];
                const uint8_t r = (p >> (8*0) & 0xFF);
                const uint8_t g = (p >> (8*1) & 0xFF);
                const uint8_t b = (p >> (8*2) & 0xFF);
                const uint8_t a = (p >> (8*3) & 0xFF);
                if (a == 0xFF) {
                    for (uint32_t ii = 0; ii < uw; ii += 1) {
                        fprintf(out, "%c%c%c", r, g, b);
                    }
                } else {
                    for (uint32_t ii = 0; ii < uw; ii += 1) {
                        fprintf(out, "%c%c%c", 0, 0, 0);
                    }
                }
            }
        }
    }
}

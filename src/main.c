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

void gen(FILE *out, const uint32_t *canvas, uint32_t w, uint32_t h, uint32_t stride);

#define FIXONE 0x8
#define WIDTH 32
#define HEIGHT 24
#define UPSCALE 0x10

void distmap_to_canvas(
    const int32_t *distmap, uint32_t w, uint32_t h, uint32_t stride,
    uint32_t *canvas, uint32_t uw, uint32_t uh, uint32_t cstride
) {
    int32_t max_neg = -1;
    int32_t max_pos = -1;
    for (uint32_t j = 0; j < h; j += 1) {
        for (uint32_t i = 0; i < w; i += 1) {
            const int32_t d = distmap[j*stride + i];
            if (d <= 0 && max_neg < -d) {
                max_neg = -d;
            }
            if (0 <= d && max_pos < d) {
                max_pos = d;
            }
        }
    }

    assert(0 < max_neg);
    assert(0 < max_pos);

    const int32_t border = FIXONE*5/8;
    for (uint32_t j = 0; j < h; j += 1) {
        for (uint32_t i = 0; i < w; i += 1) {
            const int32_t d = distmap[j*stride + i];

            uint8_t r = 0;
            uint8_t g = 0;
            uint8_t b = 0;

            if (d < 0) {
                b = ((uint32_t) (-d)) * 0xFF / max_neg;
            } else {
                r = ((uint32_t) d) * 0xFF / max_pos;
            }
            if (-border < d && d < border) {
                g = 0xFF;
            }
            const uint32_t color = rgb(r, g, b);

            for (uint32_t jj = 0; jj < uh; jj += 1) {
                for (uint32_t ii = 0; ii < uw; ii += 1) {
                    canvas[((j*uh)+jj)*cstride + ((i*uw)+ii)] = color;
                }
            }
        }
    }
}

int main(void) {
    FILE *out = fopen("img.ppm", "w");

    const uint32_t w = WIDTH;
    const uint32_t h = HEIGHT;
    int32_t distmap[WIDTH*HEIGHT];
    uint32_t canvas[WIDTH*UPSCALE*HEIGHT*UPSCALE];

    const int32_t cx = 0*FIXONE;
    const int32_t cy = 0*FIXONE;
    const int32_t r = 8*FIXONE;

    for (uint32_t j = 0; j < h; j += 1) {
        const int32_t py = (j - (h/2))*FIXONE;
        for (uint32_t i = 0; i < w; i += 1) {
            const int32_t px = (i - (w/2))*FIXONE;
            const int32_t dist = zdf_circle(cx, cy, r, px, py);
            distmap[j*w + i] = dist;
        }
    }

    distmap_to_canvas(distmap, w, h, w, canvas, UPSCALE, UPSCALE, w*UPSCALE);

    gen(out, canvas, w*UPSCALE, h*UPSCALE, w*UPSCALE);
    fclose(out);

    return 0;
}

#define ZDF_IMPLEMENTATION
#include "zdf.h"

// PBM serializer (to P6)
void gen(FILE *out, const uint32_t *canvas, uint32_t w, uint32_t h, uint32_t stride) {
    fprintf(out, "P6 %d %d 255\n", w, h);
    for (uint32_t j = 0; j < h; j += 1) {
        for (uint32_t i = 0; i < w; i += 1) {
            const uint32_t p = canvas[j*stride + i];
            const uint8_t r = (p >> (8*0) & 0xFF);
            const uint8_t g = (p >> (8*1) & 0xFF);
            const uint8_t b = (p >> (8*2) & 0xFF);
            const uint8_t a = (p >> (8*3) & 0xFF);
            if (a == 0xFF) {
                fprintf(out, "%c%c%c", r, g, b);
            } else {
                fprintf(out, "%c%c%c", 0, 0, 0);
            }
        }
    }
}

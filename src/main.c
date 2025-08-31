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

#define FIXONE 0x10
#define WIDTH 32
#define HEIGHT 24
#define UPSCALE 0x10

void distmap_to_canvas(
    const int32_t *distmap, uint32_t w, uint32_t h, uint32_t stride,
    uint32_t *canvas, uint32_t uw, uint32_t uh, uint32_t cstride,
    uint32_t border
) {
    uint32_t max_neg = 0;
    uint32_t max_pos = 0;
    const uint32_t min = 0x10;
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
            const int32_t d = distmap[j*stride + i];
            const uint32_t ud = (uint32_t) (d < 0 ? -d : d);

            uint8_t r = 0;
            uint8_t g = 0;
            uint8_t b = 0;

            if (d < 0) {
                b = (ud * (0xFF - min) / max_neg) + min;
            } else {
                r = (ud * (0xFF - min) / max_pos) + min;
            }
            if (ud < border) {
                g = ((border - ud) * (0xFF - min) / border) + min;
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

typedef struct {
    int32_t xoff;
    int32_t yoff;
    int32_t mul;
    int32_t div;
} Camera2D;

static
uint32_t camera_to_world_i(const Camera2D camera, uint32_t i) {
    return ((((int32_t) i)*FIXONE) - camera.xoff) * camera.mul / camera.div;
}

static
uint32_t camera_to_world_j(const Camera2D camera, uint32_t j) {
    return ((((int32_t) j)*FIXONE) - camera.yoff) * camera.mul / camera.div;
}

int main(void) {
    const uint32_t w = WIDTH;
    const uint32_t h = HEIGHT;
    int32_t distmap[WIDTH*HEIGHT];
    uint32_t canvas[WIDTH*UPSCALE*HEIGHT*UPSCALE];

    const Camera2D camera = {
        .xoff = (w - 1)*FIXONE/2,
        .yoff = (h - 1)*FIXONE/2,
        .mul = 3,
        .div = 4,
    };
    const uint32_t border = FIXONE*2/8;

    const int32_t cx = 0*FIXONE;
    const int32_t cy = 0*FIXONE;
    const int32_t r = 8*FIXONE;

    for (uint32_t j = 0; j < h; j += 1) {
        const int32_t py = camera_to_world_j(camera, j);
        for (uint32_t i = 0; i < w; i += 1) {
            const int32_t px = camera_to_world_i(camera, i);
            const int32_t dist = zdf_circle(cx, cy, r, px, py);
            distmap[j*w + i] = dist;
        }
    }

    distmap_to_canvas(
        distmap, w, h, w,
        canvas, UPSCALE, UPSCALE, w*UPSCALE,
        border
    );

    FILE *out = fopen("img.ppm", "w");
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

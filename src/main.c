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

#define ZDF_INT int32_t
#define ZDF_LONG int64_t
#include "zdf.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define ARRLEN(x) ((sizeof(x))/(sizeof(x[0])))

uint16_t canvas_to_palette(const uint32_t canvas[], uint32_t w, uint32_t h, uint32_t stride, uint8_t indexes[], uint32_t idxsstride, uint32_t palette[], uint32_t palette_len);
void canvas_to_pbm6(FILE *out, const uint32_t *canvas, uint32_t w, uint32_t h, uint32_t stride, uint32_t uw, uint32_t uh);
void paletteidxs_to_pbm6(FILE *out, const uint8_t indexes[], uint32_t w, uint32_t h, uint32_t stride, uint32_t uw, uint32_t uh, uint16_t palette_len);

#define FIXONE 0x10
#define WIDTH 32
#define HEIGHT 24
#define UPSCALE 0x10

#define PALETTE_LEN 0x100

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
            const uint32_t ud = zdf_iabs(d);

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

void gradmap_to_canvas(
    const ZdfVec2 *gradmap, uint32_t w, uint32_t h, uint32_t stride,
    uint32_t *canvas
) {
    for (uint32_t j = 0; j < h; j += 1) {
        for (uint32_t i = 0; i < w; i += 1) {
            const int32_t idx = j*stride + i;

            const ZdfVec2 grad = gradmap[idx];
            const uint32_t ugx = zdf_iabs(grad.x);
            const uint32_t ugy = zdf_imax(grad.y, 0);

            uint8_t r = 0;
            uint8_t g = ugy * 0xFF / FIXONE;
            uint8_t b = 0;

            if (grad.x < 0) {
                b = ugx * 0xFF / FIXONE;
            } else {
                r = ugx * 0xFF / FIXONE;
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

int32_t sdf_dist(const ZdfCircle circles[], uint32_t circles_len, const ZdfBox boxes[], uint32_t boxes_len, const ZdfLine lines[], uint32_t lines_len, ZdfVec2 p);
int32_t sdf_dist_grad(const ZdfCircle circles[], uint32_t circles_len, const ZdfBox boxes[], uint32_t boxes_len, const ZdfLine lines[], uint32_t lines_len, ZdfVec2 p, ZdfVec2 *out_grad);

int main(void) {
    const uint32_t w = WIDTH;
    const uint32_t h = HEIGHT;
    int32_t *distmap = malloc(WIDTH*HEIGHT*sizeof(*distmap));
    assert(distmap);
    ZdfVec2 *gradmap = malloc(WIDTH*HEIGHT*sizeof(*gradmap));
    assert(gradmap);
    uint32_t *canvas = malloc(WIDTH*HEIGHT*sizeof(*canvas));
    assert(canvas);
    uint32_t *palette = malloc(PALETTE_LEN*sizeof(*palette));
    assert(palette);
    uint8_t *palette_idxs = malloc(WIDTH*HEIGHT*sizeof(*palette_idxs));
    assert(palette_idxs);

    const Camera2D camera = (Camera2D){
        .off = (ZdfVec2){
            .x = (w - 1)*FIXONE/2,
            .y = (h - 1)*FIXONE/2,
        },
        .mul = 16,
        .div = h,
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
    const ZdfBox boxes[] = {
        (ZdfBox){
            .c = (ZdfVec2){
                .x = - 14*FIXONE/2,
                .y = - 1*FIXONE,
            },
            .r = (ZdfVec2){
                .x = 3*FIXONE/2,
                .y = 13*FIXONE/2,
            },
        },
    };
    const ZdfLine lines[] = {
        (ZdfLine){
            .off = 17*FIXONE/2,
            .n = zdf_ivnormal((ZdfVec2){
                .x = - 1*FIXONE,
                .y = 1*FIXONE,
            }, FIXONE),
        },
        (ZdfLine){
            .off = 17*FIXONE/2,
            .n = zdf_ivnormal((ZdfVec2){
                .x = - 1*FIXONE,
                .y = - 1*FIXONE,
            }, FIXONE),
        },
    };

    for (uint32_t j = 0; j < h; j += 1) {
        for (uint32_t i = 0; i < w; i += 1) {
            const uint32_t idx = j*w + i;
            const ZdfVec2 p = camera_to_world(camera, i, j);
            const int32_t dist = sdf_dist_grad(circles, ARRLEN(circles), boxes, ARRLEN(boxes), lines, ARRLEN(lines), p, &gradmap[idx]);
            distmap[idx] = dist;
        }
    }

    if (1) {
        gradmap_to_canvas(
            gradmap, w, h, w,
            canvas
        );
    } else {
        distmap_to_canvas(
            distmap, w, h, w,
            canvas,
            border
        );
    }

    {
        FILE *out = fopen("img.ppm", "w");
        canvas_to_pbm6(out, canvas, w, h, w, UPSCALE, UPSCALE);
        fclose(out);
    }

    const uint16_t palette_len = canvas_to_palette(canvas, w, h, w, palette_idxs, w, palette, PALETTE_LEN);
    assert(0 < palette_len);

    {
        FILE *out = fopen("palette.ppm", "w");
        paletteidxs_to_pbm6(out, palette_idxs, w, h, w, UPSCALE, UPSCALE, palette_len);
        fclose(out);
    }

    return 0;
}

int32_t sdf_dist(const ZdfCircle circles[], uint32_t circles_len, const ZdfBox boxes[], uint32_t boxes_len, const ZdfLine lines[], uint32_t lines_len, ZdfVec2 p) {
    assert(0 < circles_len);
    int32_t dist = zdf_circle(circles[0], p);

    for (uint32_t k = 1; k < circles_len; k += 1) {
        const int32_t d = zdf_circle(circles[k], p);
        dist = zdf_imin(d, dist);
    }
    for (uint32_t k = 0; k < boxes_len; k += 1) {
        const int32_t d = zdf_box(boxes[k], p);
        dist = zdf_imin(d, dist);
    }
    for (uint32_t k = 0; k < lines_len; k += 1) {
        const int32_t d = zdf_line(lines[k], p, FIXONE);
        dist = zdf_imin(d, dist);
    }
    return dist;
}

int32_t sdf_dist_grad(const ZdfCircle circles[], uint32_t circles_len, const ZdfBox boxes[], uint32_t boxes_len, const ZdfLine lines[], uint32_t lines_len, ZdfVec2 p, ZdfVec2 *out_grad) {
    if (!out_grad) {
        return sdf_dist(circles, circles_len, boxes, boxes_len, lines, lines_len, p);
    }
    assert(0 < circles_len);
    int32_t dist = zdf_circle(circles[0], p);
    ZdfVec2 grad = zdf_circle_grad(circles[0], p, FIXONE);

    for (uint32_t k = 1; k < circles_len; k += 1) {
        const int32_t d = zdf_circle(circles[k], p);
        if (d < dist) {
            dist = d;
            grad = zdf_circle_grad(circles[k], p, FIXONE);
        }
    }
    for (uint32_t k = 0; k < boxes_len; k += 1) {
        const int32_t d = zdf_box(boxes[k], p);
        if (d < dist) {
            dist = d;
            grad = zdf_box_grad(boxes[k], p, FIXONE);
        }
    }
    for (uint32_t k = 0; k < lines_len; k += 1) {
        const int32_t d = zdf_line(lines[k], p, FIXONE);
        if (d < dist) {
            dist = d;
            grad = zdf_line_grad(lines[k], p);
        }
    }
    *out_grad = grad;
    return dist;
}

#define ZDF_IMPLEMENTATION
#define ZDF_ASSERT assert
#define ZDF_NOTRUST_LISQRT
#include "zdf.h"

uint16_t canvas_to_palette(
    const uint32_t canvas[], uint32_t w, uint32_t h, uint32_t stride,
    uint8_t indexes[], uint32_t idxsstride,
    uint32_t palette[], uint32_t palette_len
) {
    uint16_t size = 0;
    for (uint32_t j = 0; j < h; j += 1) {
        for (uint32_t i = 0; i < w; i += 1) {
            const uint32_t color = canvas[j*stride + i];

            uint16_t pidx = 0;
            if (0 < size) {
                // Binary search
                uint16_t low = 0;
                uint16_t high = size;
                assert(low == 0 || palette[low - 1] < color);
                assert(high == size || color < palette[high]);
                while (low < high) {
                    const uint16_t ii = (low + high)/2;
                    if (palette[ii] < color) {
                        low = ii + 1;
                    } else if (color < palette[ii]) {
                        high = ii;
                    } else {
                        low = ii;
                        break;
                    }
                    assert(low == 0 || palette[low - 1] < color);
                    assert(high == size || color < palette[high]);
                }
                if (low == high) {
                    pidx = low;
                } else {
                    continue;
                }
            }

            assert(size < palette_len);
            palette[size] = color;
            size += 1;
            for (uint16_t ii = size - 1; pidx < ii; ii -= 1) {
                assert(palette[ii] <= palette[ii - 1]);
                const uint32_t t = palette[ii - 1];
                palette[ii - 1] = palette[ii];
                palette[ii] = t;
            }
            assert(palette[pidx] == color);
        }
    }

    for (uint32_t i = 0; i + 1 < size; i += 1) {
        assert(palette[i] < palette[i + 1]);
    }

    for (uint32_t j = 0; j < h; j += 1) {
        for (uint32_t i = 0; i < w; i += 1) {
            const uint32_t color = canvas[j*stride + i];

            uint32_t pidx = 0;
            { // Binary Search
                uint32_t low = 0;
                uint32_t high = size;
                assert(low == 0 || palette[low - 1] < color);
                assert(high == size || color < palette[high]);
                while (low < high) {
                    const uint32_t ii = (low + high)/2;
                    if (palette[ii] < color) {
                        low = ii + 1;
                    } else if (color < palette[ii]) {
                        high = ii;
                    } else {
                        low = ii;
                        break;
                    }
                    assert(low == 0 || palette[low - 1] < color);
                    assert(high == size || color < palette[high]);
                }
                pidx = low;
            }
            indexes[j*idxsstride + i] = pidx;
        }
    }
    return size;
}

void canvas_to_pbm6(FILE *out, const uint32_t canvas[], uint32_t w, uint32_t h, uint32_t stride, uint32_t uw, uint32_t uh) {
    fprintf(out, "P6 %d %d 255\n", w*uw, h*uh);
    for (uint32_t j = 0; j < h; j += 1) {
        for (uint32_t jj = 0; jj < uh; jj += 1) {
            for (uint32_t i = 0; i < w; i += 1) {
                const uint32_t p = canvas[j*stride + i];
                const uint8_t r = (p >> (8*0) & 0xFF);
                const uint8_t g = (p >> (8*1) & 0xFF);
                const uint8_t b = (p >> (8*2) & 0xFF);
                const uint8_t a = (p >> (8*3) & 0xFF);
                const uint8_t color[3] = {
                    r * a / 0xFF,
                    g * a / 0xFF,
                    b * a / 0xFF,
                };
                for (uint32_t ii = 0; ii < uw; ii += 1) {
                    fwrite(color, sizeof(color[0]), ARRLEN(color), out);
                }
            }
        }
    }
}

void paletteidxs_to_pbm6(FILE *out, const uint8_t indexes[], uint32_t w, uint32_t h, uint32_t stride, uint32_t uw, uint32_t uh, uint16_t palette_len) {
    fprintf(out, "P6 %d %d 255\n", w*uw, h*uh);
    for (uint32_t j = 0; j < h; j += 1) {
        for (uint32_t jj = 0; jj < uh; jj += 1) {
            for (uint32_t i = 0; i < w; i += 1) {
                const uint32_t idx = indexes[j*stride + i];

                const uint32_t p = idx * 0x00FFFFFF / palette_len;
                const uint8_t color[3] = {
                    (p >> (8*0) & 0xFF),
                    (p >> (8*1) & 0xFF),
                    (p >> (8*2) & 0xFF),
                };
                for (uint32_t ii = 0; ii < uw; ii += 1) {
                    fwrite(color, sizeof(color[0]), ARRLEN(color), out);
                }
            }
        }
    }
}

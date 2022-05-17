#ifndef _GRAPHICS_H
#define _GRAPHICS_H
#include "pico.h"

#define GRAPHICS_USE_SUB_WINDOW 1
#define GRAPHICS_BOUNDARY_CHECK 1

typedef enum {
	rgb_8  = 1,
    rgb_16 = 2,
    rgb_24 = 3
} graphics_bppx;

typedef struct graphic_ctx graphic_ctx_t;

struct graphic_ctx {
    uint16_t         width;
    uint16_t         height;
    uint16_t         x;
    uint16_t         y;
    void            *video_buffer;
    graphics_bppx   bppx;
    graphic_ctx_t   *parent;
};

void put_pixel(const graphic_ctx_t *ctx, uint x, uint y, uint color);
uint get_pixel(const graphic_ctx_t *ctx, uint x, uint y);
void fill_rect(const graphic_ctx_t *ctx, uint x0, uint y0, uint x1, uint y1, uint color);
void draw_circle(const graphic_ctx_t *ctx, uint xc, uint yc, uint radius, uint color);

#endif
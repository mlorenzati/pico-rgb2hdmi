#include "graphics.h"

static inline void *get_buffer(const graphic_ctx_t *ctx, uint x, uint y) {
    #ifdef GRAPHICS_USE_SUB_WINDOW
    while (ctx->parent != NULL) {
        x += ctx->x;
        y += ctx->y;
        ctx = ctx->parent;
    }
    #endif
    return ctx->video_buffer + (x + (y * ctx->width)) * ctx->bppx;
}

void put_pixel(const graphic_ctx_t *ctx, uint x, uint y, uint color) {
    #ifdef GRAPHICS_BOUNDARY_CHECK
    if (x >= ctx->width || y >= ctx->height) {
        return;
    }
    #endif

    void *buffer = get_buffer(ctx, x, y);
    switch (ctx->bppx) {
        case rgb_8:  *((uint8_t  *)buffer) = color & 0xFF;     break;
        case rgb_16: *((uint16_t *)buffer) = color & 0xFFFF;   break;
        case rgb_24: *((uint32_t *)buffer) = color & 0xFFFFFF; break;
    }
}

uint get_pixel(const graphic_ctx_t *ctx, uint x, uint y) {
    #ifdef GRAPHICS_BOUNDARY_CHECK
    if (x >= ctx->width || y >= ctx->height) {
        return 0;
    }
    #endif

    void *buffer = get_buffer(ctx, x, y);
    switch (ctx->bppx) {
        case rgb_8:  return *((uint8_t  *)buffer) & 0xFF;
        case rgb_16: return *((uint16_t *)buffer) & 0xFFFF;
        case rgb_24: return *((uint32_t *)buffer) & 0xFFFFFF;
    }
    return 0;
}

void fill_rect(const graphic_ctx_t *ctx, uint x0, uint y0, uint x1, uint y1, uint color) {
	for (uint x = x0; x <= x1; ++x)
		for (uint y = y0; y <= y1; ++y)
			put_pixel(ctx, x, y, color);
}

void draw_bresen(const graphic_ctx_t *ctx, uint xc, uint yc, uint x, uint y, uint color) {
    put_pixel(ctx, xc + x, yc + y, color);
    put_pixel(ctx, xc - x, yc + y, color);
    put_pixel(ctx, xc + x, yc - y, color);
    put_pixel(ctx, xc - x, yc - y, color);
    put_pixel(ctx, xc + y, yc + x, color);
    put_pixel(ctx, xc - y, yc + x, color);
    put_pixel(ctx, xc + y, yc - x, color);
    put_pixel(ctx, xc - y, yc - x, color);
}

void draw_circle(const graphic_ctx_t *ctx, uint xc, uint yc, uint radius, uint color) {
    int x = 0, y = radius;
    int d = 3 - 2 * radius;
    draw_bresen(ctx, xc, yc, x, y, color);
    while (y >= x)
    {
        x++;
        if (d > 0) {
            y--;
            d = d + 4 * (x - y) + 10;
        }
        else {
            d = d + 4 * x + 6;
        }
            
        draw_bresen(ctx, xc, yc, x, y, color);
    }
}
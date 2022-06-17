#include "graphics.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "font_8x8.h"

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

void draw_line(const graphic_ctx_t *ctx, uint x0, uint y0, uint x1, uint y1, uint color) {
  signed int dx = abs(x1 - x0);
  signed int sx = x0 < x1 ? 1 : -1;
  signed int dy = -abs(y1 - y0);
  signed int sy = y0 < y1 ? 1 : -1; 
  signed int err = dx + dy, e2; /* error value e_xy */
 
  for (;;) {  /* loop */
    put_pixel (ctx, x0, y0, color);
    if (x0 == x1 && y0 == y1) break;
    e2 = 2 * err;
    if (e2 >= dy) { err += dy; x0 += sx; } /* e_xy+e_x > 0 */
    if (e2 <= dx) { err += dx; y0 += sy; } /* e_xy+e_y < 0 */
  }
}
void draw_rect(const graphic_ctx_t *ctx, uint x0, uint y0, uint width, uint height, uint color) {
    uint x1 = x0 + width;
    uint y1 = y0 + height;
    draw_line(ctx, x0, y0, x1, y0, color);
    draw_line(ctx, x0, y0, x0, y1, color);
    draw_line(ctx, x0, y1, x1, y1, color);
    draw_line(ctx, x1, y0, x1, y1, color);
}

void fill_rect(const graphic_ctx_t *ctx, uint x0, uint y0, uint width, uint height, uint color) {
    uint x1 = x0 + width;
    uint y1 = y0 + height;
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

void draw_circle(const graphic_ctx_t *ctx, uint xc, uint yc, signed int radius, uint color) {
    int x = 0, y = abs(radius);
    int d = 3 - 2 * y;
    if (radius == 0) { return ; }

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
    if (radius < 0) {
        draw_circle(ctx, xc, yc, radius + 1, color);
    }
}



void draw_text(const graphic_ctx_t *ctx, uint x0, uint y0, uint fg_color, uint bg_color, bool word_wrap, const char *text) {
    if (y0 >= ctx->height) { return; }
    uint y_max = (y0 + GRAPHICS_FONT_SIZE) >= ctx->height ? ctx->height - 1 : y0 + GRAPHICS_FONT_SIZE;
    const char *ptr = NULL;
    char c = 0;
    uint xbase = 0;

    for (int y = y0; y < y_max; ++y) {
        xbase = x0;
		ptr = text; 
        while ((c = *ptr++) > 0 && (c != '\n') && (word_wrap ? (xbase + GRAPHICS_FONT_SIZE) < ctx->width : xbase < ctx->width)) {
            uint8_t font_bits = font_8x8[(c - FONT_FIRST_ASCII) + (y - y0) * FONT_N_CHARS];
            for (int i = 0; i < GRAPHICS_FONT_SIZE; ++i) {
                bool pixel = font_bits & (1u << i);
                if ((bg_color != fg_color) || pixel) {
                    put_pixel(ctx, xbase + i, y, pixel ? fg_color : bg_color);
                }
            }
            xbase += GRAPHICS_FONT_SIZE;
        }
    }
    if (word_wrap && ((xbase + GRAPHICS_FONT_SIZE) >= ctx->width)) {
        c = '\n';
        ptr = text + (ctx->width - x0) / GRAPHICS_FONT_SIZE;
        if (*ptr == '\n') { ptr++; }
    }
    if (c == '\n') {
        draw_text(ctx, x0, y0 + GRAPHICS_FONT_SIZE, fg_color, bg_color, word_wrap, ptr);
    }
}

void draw_textf(const graphic_ctx_t *ctx, uint x0, uint y0, uint fg_color, uint bg_color, bool word_wrap, const char *fmt, ...) {
    char buf[128];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, 128, fmt, args);
	draw_text(ctx, x0, y0, fg_color, bg_color, word_wrap, buf);
	va_end(args); 
}
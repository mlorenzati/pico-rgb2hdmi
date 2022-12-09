#include "graphics.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "color.h"

#ifdef FONT_8X8
#include "font_8x8.h"
#elif  FONT_C64_8X8
#include "font_C64_8x8.h"
#endif

graphic_ctx_t get_sub_graphic_ctx(const graphic_ctx_t *parent, uint x, uint y, uint width, uint height) {
    graphic_ctx_t ctx = {
        .video_buffer = NULL,
        .bppx = parent->bppx,
        .parent = parent,
        .width = width,
	    .height = height,
	    .x = x,
	    .y = y
    };
    return ctx;
}

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

    bppx_put_color(ctx->bppx, get_buffer(ctx, x, y), color);
}

uint get_pixel(const graphic_ctx_t *ctx, uint x, uint y) {
    #ifdef GRAPHICS_BOUNDARY_CHECK
    if (x >= ctx->width || y >= ctx->height) {
        return 0;
    }
    #endif

    return bppx_get_color(ctx->bppx, get_buffer(ctx, x, y));
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
    uint x1 = x0 + width - 1;
    uint y1 = y0 + height - 1;
    draw_line(ctx, x0, y0, x1, y0, color);
    draw_line(ctx, x0, y0, x0, y1, color);
    draw_line(ctx, x0, y1, x1, y1, color);
    draw_line(ctx, x1, y0, x1, y1, color);
}

void fill_rect(const graphic_ctx_t *ctx, uint x0, uint y0, uint width, uint height, uint color) {
    uint x1 = x0 + width;
    uint y1 = y0 + height;
	for (uint x = x0; x < x1; ++x)
		for (uint y = y0; y < y1; ++y)
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

void fill_circle(const graphic_ctx_t *ctx, uint xc, uint yc, signed int radius, uint fg_color, uint bg_color) {
    draw_circle(ctx, xc, yc, radius, fg_color);
    draw_flood(ctx, xc, yc, bg_color, fg_color, true);
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

bool shoud_fill(const graphic_ctx_t *ctx, uint x, uint y, uint fill_color, uint logic_color, bool invert) {
 #ifdef GRAPHICS_BOUNDARY_CHECK
    if (x >= ctx->width || y >= ctx->height) {
        return false;
    }
#endif
    uint color = get_pixel(ctx, x, y);
    return (color != fill_color) && ((color == logic_color) ^ invert);
}

void graphic_flood_queue_init(graphic_flood_queue_t *queue) {
    queue->head = 0;
    queue->tail = 0;
}

bool graphic_flood_queue_is_empty(graphic_flood_queue_t *queue) {
    return queue->head == queue->tail;
}

void graphic_flood_queue_add(graphic_flood_queue_t *queue, uint x1, uint x2, uint y, int dy) {
    queue->x1[queue->head] = x1;
    queue->x2[queue->head] = x2;
    queue->y[queue->head]  = y;
    queue->dy[queue->head] = dy;
    queue->head++;
    queue->head %= GRAPHICS_FLOOD_DEPTH;
    if (queue->head == queue->tail) {
        queue->tail++;
        queue->tail %= GRAPHICS_FLOOD_DEPTH;
    }
}
void graphic_flood_queue_pop(graphic_flood_queue_t *queue, uint *x1, uint *x2, uint *y, int *dy) {
    *x1 = queue->x1[queue->tail];
    *x2 = queue->x2[queue->tail];
    *y  =  queue->y[queue->tail];
    *dy = queue->dy[queue->tail];
    queue->tail++;
    queue->tail %= GRAPHICS_FLOOD_DEPTH;
}

void draw_flood(const graphic_ctx_t *ctx, uint x, uint y, uint fill_color, uint logic_color, bool invert) {
    if (!shoud_fill(ctx, x, y, fill_color, logic_color, invert)) {
        return;
    }

    graphic_flood_queue_t queue;
    graphic_flood_queue_init(&queue);
    graphic_flood_queue_add(&queue, x, x, y, 1);
    graphic_flood_queue_add(&queue, x, x, y - 1, -1);

    uint x1, x2;
    int dy;
    while (!graphic_flood_queue_is_empty(&queue)) {
        graphic_flood_queue_pop(&queue, &x1, &x2, &y, &dy);
        x = x1;
        if (shoud_fill(ctx, x, y, fill_color, logic_color, invert)) {
            while (shoud_fill(ctx, x - 1, y, fill_color, logic_color, invert)) {
                put_pixel(ctx, x - 1, y, fill_color);
                x--;
            }
        }
        if (x < x1) {
            graphic_flood_queue_add(&queue, x, x1 - 1, y - dy, -dy);
        }
        while (x1 <= x2) {
            while (shoud_fill(ctx, x1, y, fill_color, logic_color, invert)) {
                put_pixel(ctx, x1, y, fill_color);
                x1++;
                graphic_flood_queue_add(&queue, x, x1 - 1, y + dy, dy);
                if (x1 - 1 > x2) {
                    graphic_flood_queue_add(&queue, x2 + 1, x1 - 1, y - dy, -dy);
                }
            }
            x1++;
            while (x1 < x2 && !shoud_fill(ctx, x1, y, fill_color, logic_color, invert)) {
                x1++;
            }
            x = x1;
        }
    }
}
#ifndef _GRAPHICS_H
#define _GRAPHICS_H
#include "pico.h"

#define GRAPHICS_USE_SUB_WINDOW 1
#define GRAPHICS_BOUNDARY_CHECK 1
#define GRAPHICS_FONT_SIZE      8

typedef enum {
	rgb_8  = 1,
    rgb_16 = 2,
    rgb_24 = 3
} graphics_bppx;

#define GRAPHICS_FLOOD_DEPTH 128
typedef struct graphic_flood_queue {
    uint x1[GRAPHICS_FLOOD_DEPTH];
    uint x2[GRAPHICS_FLOOD_DEPTH];
    uint y[GRAPHICS_FLOOD_DEPTH];
    int dy[GRAPHICS_FLOOD_DEPTH];
    uint16_t head,tail;
} graphic_flood_queue_t;

typedef struct graphic_ctx graphic_ctx_t;

struct graphic_ctx {
    uint16_t             width;
    uint16_t             height;
    uint16_t             x;
    uint16_t             y;
    void                *video_buffer;
    graphics_bppx        bppx;
    const graphic_ctx_t *parent;
};

#define GET_RGB8_BUFFER(x)  ((uint8_t *)x)
#define GET_RGB16_BUFFER(x) ((uint16_t *)x)

graphic_ctx_t get_sub_graphic_ctx(const graphic_ctx_t *parent, uint x, uint y, uint width, uint height);

uint8_t bppx_to_int(graphics_bppx bppx);
void put_pixel(const graphic_ctx_t *ctx, uint x, uint y, uint color);
uint get_pixel(const graphic_ctx_t *ctx, uint x, uint y);
void draw_line(const graphic_ctx_t *ctx, uint x0, uint y0, uint x1, uint y1, uint color);
void draw_rect(const graphic_ctx_t *ctx, uint x0, uint y0, uint width, uint height, uint color);
void fill_rect(const graphic_ctx_t *ctx, uint x0, uint y0, uint width, uint height, uint color);
void draw_circle(const graphic_ctx_t *ctx, uint xc, uint yc, signed int radius, uint color);
void draw_text(const graphic_ctx_t *ctx, uint x0, uint y0, uint fg_color, uint bg_color, bool word_wrap, const char *text);
void draw_textf(const graphic_ctx_t *ctx, uint x0, uint y0, uint fg_color, uint bg_color, bool word_wrap, const char *fmt, ...);
void draw_flood(const graphic_ctx_t *ctx, uint x, uint y, uint fill_color, uint logic_color, bool invert);
// TODO: text, line, rect
#endif
#ifndef _MENU_H
#define _MENU_H
#include "graphics.h"

typedef struct menu_base {
   const graphic_ctx_t *ctx;
   uint x, y;
   uint width, height;
   const void *data;
} menu_base_t;

typedef void (*menu_cb_draw_t)(menu_base_t *data);

typedef struct menu_object {
   menu_base_t    base;
   menu_cb_draw_t draw;
} menu_object_t;

menu_object_t menu_create_window(const graphic_ctx_t *ctx, uint x, uint y, uint width, uint height); 
menu_object_t menu_create_button(const graphic_ctx_t *ctx, uint x, uint y, uint width, uint height, const char *text); 
#endif
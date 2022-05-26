#include "menu.h"

void menu_draw_window(menu_base_t *base) {
    uint color = 0b0000011111100000;
    draw_rect(base->ctx, base->x, base->y, base->width, base->height, color);
}

void menu_draw_button(menu_base_t *base) {
    uint color1 = 0b0000011111100000;
    uint color2 = 0b0101010101010101;
    draw_rect(base->ctx, base->x, base->y, base->width, base->height, color1);
    draw_textf(base->ctx, (base->x)+2, (base->y)+2, color1, color2, (const char *) base->data);
}

menu_base_t menu_create_base(const graphic_ctx_t *ctx, uint x, uint y, uint width, uint height) {
    menu_base_t   base;
    base.ctx = ctx;
    base.x = x;
    base.y = y;
    base.width = width;
    base.height = height;

    return base;
}

menu_object_t menu_create_window(const graphic_ctx_t *ctx, uint x, uint y, uint width, uint height) {
    menu_object_t window;
    window.base = menu_create_base(ctx, x, y, width, height);
    window.draw = menu_draw_window;

    return window;
}

menu_object_t menu_create_button(const graphic_ctx_t *ctx, uint x, uint y, uint width, uint height, const char *text) {
    menu_object_t button;
    button.base = menu_create_base(ctx, x, y, width, height);
    button.base.data = (const void *)text;
    button.draw = menu_draw_button;

    return button;
}
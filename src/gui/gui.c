#include "gui.h"
#include <string.h>

#define GUI_BAR_WIDTH      6

gui_object_t gui_create_object(const graphic_ctx_t *ctx, uint x, uint y, uint width, uint height, 
    gui_color_list_t colors, const uint8_t *data, gui_cb_draw_t draw_cb) {
    gui_object_t obj = {
        .base = {
            .ctx = ctx,
            .x = x,
            .y = y,
            .width = width,
            .height = height,
            .colors = colors,
            .status = {
                .enabled = 1
            },
            .data = data
        },
        .draw = draw_cb
    };
    return obj;
}

void gui_draw_window(gui_base_t *base) {
    draw_rect(base->ctx, base->x, base->y, base->width, base->height, base->colors.elements[0]);
    fill_rect(base->ctx, base->x + 1, base->y + 1, base->width - 2, base->height - 2, base->colors.elements[1]);
}

void gui_draw_border(gui_base_t *base) {
    uint color_border_1 = base->status.enabled ?
        base->status.activated ? base->colors.elements[0]: base->colors.elements[2]: base->colors.elements[2];
    uint color_border_2 = base->status.enabled ? 
        base->status.activated ? base->colors.elements[2]: base->colors.elements[0]: base->colors.elements[2];
    uint color_bg =   base->status.activated ?
        base->colors.elements[4] : base->status.focused ? base->colors.elements[5] : base->colors.elements[1];

    draw_line(base->ctx, base->x, base->y, base->x + base->width, base->y, color_border_1);
    draw_line(base->ctx, base->x, base->y, base->x, base->y + base->height - 1, color_border_1);
    draw_line(base->ctx, base->x, base->y + base->height, base->x + base->width, base->y + base->height, color_border_2);
    draw_line(base->ctx, base->x + base->width, base->y + 1, base->x + base->width, base->y + base->height, color_border_2);
    fill_rect(base->ctx, base->x + 1, base->y + 1, base->width - 1, base->height - 1, color_bg);
}

void gui_draw_button(gui_base_t *base) {
    gui_draw_border(base);
    uint color_text = base->status.enabled ? base->colors.elements[3] : base->colors.elements[2];
    graphic_ctx_t text_ctx = get_sub_graphic_ctx(base->ctx, base->x + 1, base->y + 1, base->width - 1, base->height - 1);
    
    const char *text = (const char*) base->data; 
    uint text_width = (strlen(text) * GRAPHICS_FONT_SIZE) - 1;
    uint width = text_ctx.width > text_width ? text_ctx.width - text_width : 0;
    uint height = text_ctx.height > (GRAPHICS_FONT_SIZE - 1) ? text_ctx.height - GRAPHICS_FONT_SIZE + 1: 0;
    uint x = width >> 1;
    uint y = height >> 1;
    
    draw_textf(&text_ctx, x, y, color_text, color_text, false, text);
}

void gui_draw_slider(gui_base_t *base) {
    gui_draw_window(base);
    gui_base_t button = *base;
    uint val = *((uint *)(base->data));
    uint posx = 1 + ((base->width - 3 - GUI_BAR_WIDTH) * val) / GUI_BAR_100PERCENT;
    button.x += posx;
    button.y += 1;
    button.width = GUI_BAR_WIDTH;
    button.height -= 2;
    gui_draw_border(&button);
}
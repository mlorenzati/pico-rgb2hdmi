#include "gui.h"
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#define GUI_BAR_WIDTH      6

gui_object_t gui_create_object(const graphic_ctx_t *ctx, uint x, uint y, uint width, uint height, 
    gui_list_t *colors, const uint8_t *data, gui_cb_draw_t draw_cb) {
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
    uint *colors = (uint *)(base->colors->elements);
    draw_rect(base->ctx, base->x, base->y, base->width, base->height, colors[0]);
    fill_rect(base->ctx, base->x + 1, base->y + 1, base->width - 2, base->height - 2, colors[1]);
}

void gui_draw_focus(gui_base_t *base) {
    uint *colors = (uint *)(base->colors->elements);
    uint color_bg = base->status.activated ?
        colors[4] : base->status.focused ? colors[5] : colors[1];
    fill_rect(base->ctx, base->x, base->y, base->width, base->height, color_bg);
}

void gui_draw_border(gui_base_t *base) {
    uint *colors = (uint *)(base->colors->elements);
    uint color_border_1 = base->status.enabled ?
        base->status.activated ? colors[0]: colors[2]: colors[2];
    uint color_border_2 = base->status.enabled ? 
        base->status.activated ? colors[2]: colors[0]: colors[2];
   
    draw_line(base->ctx, base->x, base->y, base->x + base->width - 1, base->y, color_border_1);
    draw_line(base->ctx, base->x, base->y, base->x, base->y + base->height - 2, color_border_1);
    draw_line(base->ctx, base->x, base->y + base->height - 1, base->x + base->width - 1, base->y + base->height - 1, color_border_2);
    draw_line(base->ctx, base->x + base->width - 1, base->y + 1, base->x + base->width - 1, base->y + base->height - 1, color_border_2);
}

void gui_draw_text(gui_base_t *base) {
    gui_draw_focus(base);
    uint *colors = (uint *)(base->colors->elements);
    uint color_text = base->status.enabled ? colors[3] : colors[2];

    graphic_ctx_t text_ctx = get_sub_graphic_ctx(base->ctx, base->x, base->y, base->width, base->height);
    const char *text = (const char*) base->data; 
    uint text_width = (strlen(text) * GRAPHICS_FONT_SIZE) - 1;
    uint width = text_ctx.width > text_width ? text_ctx.width - text_width : 0;
    uint height = text_ctx.height > (GRAPHICS_FONT_SIZE - 1) ? text_ctx.height - GRAPHICS_FONT_SIZE + 1: 0;
    uint x = width >> 1;
    uint y = height >> 1;
    
    draw_text(&text_ctx, x, y, color_text, color_text, false, text);  
}
void gui_draw_button(gui_base_t *base) {
    gui_draw_border(base);
    gui_base_t inner_base = *base;
    inner_base.x += 1;
    inner_base.y += 1;
    inner_base.width -= 2;
    inner_base.height -= 2;
    gui_draw_text(&inner_base);
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
    button.x += 1;
    button.y += 1;
    button.width -= 2;
    button.height -= 2;
    gui_draw_focus(&button);
}

void gui_draw_label(gui_base_t *base) {
    print_delegate_caller_t print_caller = (print_delegate_caller_t)(base->data);
    gui_base_t labelBase = *base;
    
    void printer(const char * fmt, ...) {
        char buf[128];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buf, 128, fmt, args);
        labelBase.data = buf;
        gui_draw_text(&labelBase);
        va_end(args); 
    }

    print_caller(printer);
}

void gui_draw_group(gui_base_t *base) {
    gui_list_t *list = (gui_list_t *) base->data;
    gui_object_t **objects = (gui_object_t **)(list->elements);
    uint inc_pos_x = base->x;
    uint inc_pos_y = base->y;

    for (uint8_t cnt = 0; cnt < list->size; cnt++) {
        gui_object_t *object = objects[cnt];
        gui_base_t *sub_base = &(object->base);
        sub_base->x = inc_pos_x;
        sub_base->y = inc_pos_y;

        if (sub_base->y >= base->ctx->height || sub_base->x >= base->ctx->width) {
            return;
        } 
        object->draw(sub_base);
        if (base->properties.horiz_vert) {
            inc_pos_x += sub_base->width + 1;
        } else {
            inc_pos_y += sub_base->height + 1;
        }
    } 
}
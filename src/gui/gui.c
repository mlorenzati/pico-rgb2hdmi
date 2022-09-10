#include "gui.h"
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#define GUI_BAR_WIDTH      6

// ---- Base object creation ----
gui_object_t gui_create_object(const graphic_ctx_t *ctx, uint x, uint y, uint width, uint height, gui_list_t *colors, gui_properties_t props,
    const uint8_t *data, gui_cb_draw_t draw_cb) {
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
            .properties = props,
            .data = data
        },
        .draw = draw_cb
    };
    return obj;
}

// ----  Utility Functions  ----
void gui_get_start_position(gui_properties_t props, int ext_width, uint ext_height, uint in_width, uint in_height, uint *x, uint *y) {
    uint width_space = ext_width > in_width ? ext_width - in_width : 0;
    uint height_space = ext_height > in_height ? ext_height - in_height: 0;
    *x = props.alignment <= gui_align_center_down ? (width_space >> 1) : (props.alignment <= gui_align_left_down ? 0 : width_space);
    *y = props.alignment == gui_align_center || props.alignment == gui_align_left_center || props.alignment == gui_align_right_center ? 
        height_space >> 1 :  props.alignment == gui_align_center_up || props.alignment == gui_align_left_up || props.alignment == gui_align_right_up  ?
        0 : height_space;
}

// ---- Object Draw methods ----
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
    uint x, y;
    gui_get_start_position(base->properties, text_ctx.width, text_ctx.height, text_width, GRAPHICS_FONT_SIZE - 1, &x, &y);
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
    gui_object_t **s_objects = (gui_object_t **)(list->elements);
    gui_object_t *objects = (gui_object_t *)(list->elements);
    uint inc_pos_x = base->x;
    uint inc_pos_y = base->y;
    uint padding = base->properties.padding;
    bool shared = base->properties.shared;

    if (base->properties.border) {
        gui_draw_window(base);
        inc_pos_x += 1;
        inc_pos_y += 1;
    }

    for (uint8_t cnt = 0; cnt < list->size; cnt++) {
        gui_object_t *object = shared ? s_objects[cnt] : &objects[cnt];
        gui_base_t *sub_base = &(object->base);
        sub_base->x = inc_pos_x;
        sub_base->y = inc_pos_y;

        if (sub_base->y + sub_base->height >= base->y + base->height || sub_base->x + sub_base->width >= base->x + base->width) {
            return;
        } 
        object->draw(sub_base);
        if (base->properties.horiz_vert) {
            inc_pos_x += sub_base->width + padding;
        } else {
            inc_pos_y += sub_base->height + padding;
        }
    } 
}

// -- GUI event and subscriber --
gui_event_subscription_t event_subscriptions[GUI_EVENT_HANDLING_MAX];
bool gui_event_subscribe(gui_status_t status, gui_base_t *origin, gui_object_t *destination, gui_cb_on_status_t status_cv) {
    //Check the first available event slot
    gui_event_subscription_t *sel_event = NULL;
    gui_status_cast_t *n_status = (gui_status_cast_t *) &status;
    for (uint cnt = 0; cnt < GUI_EVENT_HANDLING_MAX; cnt++) {
       gui_event_subscription_t *event = &event_subscriptions[cnt];
       gui_status_cast_t *c_status = (gui_status_cast_t *) &event->status;
       //Update a subscription
       if (origin == event->origin && destination == event->destination) {
            if (n_status->word == 0) {
                origin = NULL;
                destination = NULL;
            }
            sel_event = event;
            break;
       } else if (c_status->word == 0 && sel_event == NULL) {
            sel_event = event;
            break;
       }
    }
    if (sel_event != NULL) {
        sel_event ->status = status;
        sel_event->origin = origin;
        sel_event->destination = destination;
        if (sel_event->destination != NULL) {
            sel_event->destination->status_handle = status_cv;
        }
        return true;
    }
    return false;
}

bool gui_event_unsubscribe(gui_base_t *origin, gui_object_t *destination) {
    //Search stored status
    for (uint cnt = 0; cnt < GUI_EVENT_HANDLING_MAX; cnt++) {
       gui_event_subscription_t *event = &event_subscriptions[cnt];
       gui_status_cast_t *status = (gui_status_cast_t *) &event->status;
       if (origin == event->origin && destination == event->destination) {
            status->word = 0;
            event->origin = NULL;
            event->destination = NULL;
            return true;
       }
    }
    return false;
}

void gui_event(gui_status_t status, gui_object_t *origin) {
    //Update self
    gui_status_cast_t *c_status_new = (gui_status_cast_t *) &status;
    gui_status_cast_t *c_status_old= (gui_status_cast_t *) &origin->base.status;
    gui_status_cast_t c_status_changed;
    c_status_changed.word = c_status_new->word ^ c_status_old->word;
    if (c_status_new != c_status_old) {
        *c_status_old = *c_status_new;
        origin->draw(&origin->base);
    }

    //Check subscribers of the event
    for (uint cnt = 0; cnt < GUI_EVENT_HANDLING_MAX; cnt++) {
        gui_event_subscription_t *event = &event_subscriptions[cnt];
        gui_status_cast_t *c_status_sub = (gui_status_cast_t *) &event->status;
        gui_status_cast_t c_status_match;
        c_status_match.word = c_status_sub->word & c_status_changed.word;

        if (&origin->base == event->origin && c_status_match.word > 0) {
            event->destination->status_handle(status, &origin->base, event->destination);
        }
    }

    //Data updates has to be reset after
    c_status_old->bits.data_changed = 0;
}

// GUI Utilities
uint gui_sum(gui_list_t *group, gui_properties_t props, bool width_height) {
    gui_object_t **s_objects = (gui_object_t **)(group->elements);
    gui_object_t *objects = (gui_object_t *)(group->elements);
    bool shared = props.shared;
    bool orientation = props.horiz_vert;
    uint padding = 0;

    uint width = 0;
    uint height = 0;
    for (uint8_t cnt = 0; cnt < group->size; cnt++) {
        gui_object_t *object = shared ? s_objects[cnt] : &objects[cnt];
        width  = orientation ? (width + object->base.width + padding) : (object->base.width > width ? object->base.width : width);
        height = orientation ? (object->base.height > height ? object->base.height : height) : (height + object->base.height + padding);
        padding = props.padding;
    }
    if (props.border) {
        width += 2;
        height += 2;
    }
    return width_height ? width : height;
}
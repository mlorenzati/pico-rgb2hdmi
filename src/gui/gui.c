#include "gui.h"
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#define GUI_BAR_WIDTH      6
#define GUI_BUTTON_MIN     12

// GUI Ids
const char* gui_id_window  = "window";
const char* gui_id_button  = "button";
const char* gui_id_slider  = "slider";
const char* gui_id_label   = "label";
const char* gui_id_group   = "group";
const char* gui_id_spinbox = "spinbox";

// GUI events
const gui_status_t      gui_status_focused      = { .focused = 1 };
const gui_status_t      gui_status_go_next      = { .go_next = 1 };
const gui_status_t      gui_status_go_previous  = { .go_previous = 1 };
const gui_status_t      gui_status_activated    = { .activated = 1 };
const gui_status_t      gui_status_data_changed = { .data_changed = 1 };
const gui_status_t      gui_status_add          = { .add = 1 };  
const gui_status_t      gui_status_substract    = { .substract = 1 };
const gui_status_cast_t gui_status_consumable   = { .bits = {
    .data_changed = 1
}};
const gui_status_cast_t gui_status_redrawable = { .bits = {
    .activated = 1,
    .focused = 1,
    .visible = 1,
    .enabled = 1,
    .data_changed = 1,
    .add = 1,
    .substract = 1
}};

const gui_status_cast_t gui_status_updateable = { .bits = {
    .activated = 1,
    .focused = 1,
    .visible = 1,
    .enabled = 1,
    .data_changed = 1,
    .add = 1,
    .substract = 1,
    .navigable = 1
}};

// ---- Base object creation ----
gui_object_t gui_create_object(const graphic_ctx_t *ctx, uint x, uint y, uint width, uint height, const char* id, 
    gui_list_t *colors, gui_properties_t props, const uint8_t *data, gui_cb_draw_t draw_cb) {
    gui_object_t obj = {
        .base = {
            .ctx = ctx,
            .id = id,
            .x = x,
            .y = y,
            .width = width,
            .height = height,
            .colors = colors,
            .status = {
                .enabled = 1,
                .navigable = 1
            },
            .properties = props,
            .data = data
        },
        .draw = draw_cb,
        .next = NULL,
        .previous = NULL
    };
    if (id == gui_id_group) { // We don't need a strcmp on known const keys
        gui_list_t *list = (gui_list_t *) data;
        gui_object_t **s_objects = (gui_object_t **)(list->elements);
        gui_object_t *objects = (gui_object_t *)(list->elements);

        gui_object_t *previous = props.shared ? s_objects[list->size - 1] : &objects[list->size - 1];
        for (uint8_t cnt = 0; cnt < list->size; cnt++) {
            gui_object_t *object = props.shared ? s_objects[cnt] : &objects[cnt];
            uint8_t next = (cnt + 1) % list->size;
            object->next = props.shared ? s_objects[next] : &objects[next];
            object->previous = previous;
            previous = object;
        }
    }
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
    //TODO: correct Y on multiline
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
    gui_base_t sub_base = *base;
    if (base->properties.border) {
        gui_draw_window(base);
        sub_base.x += 1;
        sub_base.y += 1;
        sub_base.width  -= 2;
        sub_base.height -= 2;
    }
    
    gui_draw_focus(&sub_base);
    uint *colors = (uint *)(sub_base.colors->elements);
    uint color_text = sub_base.status.enabled ? colors[3] : colors[2];

    graphic_ctx_t text_ctx = get_sub_graphic_ctx(sub_base.ctx, sub_base.x, sub_base.y, sub_base.width, sub_base.height);
    const char *text = (const char*) sub_base.data; 
    uint text_width = (strlen(text) * GRAPHICS_FONT_SIZE) - 1;
    uint x, y;
    gui_get_start_position(sub_base.properties, text_ctx.width, text_ctx.height, text_width, GRAPHICS_FONT_SIZE - 1, &x, &y);
    draw_text(&text_ctx, x, y, color_text, color_text, false, text);  
}

void gui_draw_button(gui_base_t *base) {
    gui_draw_border(base);
    gui_base_t inner_base = *base;
    inner_base.x += 1;
    inner_base.y += 1;
    inner_base.width -= 2;
    inner_base.height -= 2;
    inner_base.properties.border = 0;
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
        char buff[128];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buff, sizeof(buff), fmt, args);
        labelBase.data = buff;
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

        if (gui_object_overflow_group(sub_base, base)) {
            return;
        }

        gui_ref_draw(object);

        if (base->properties.horiz_vert) {
            inc_pos_x += sub_base->width + padding;
        } else {
            inc_pos_y += sub_base->height + padding;
        }
    } 
}

bool gui_object_overflow_group(gui_base_t *object, gui_base_t *group) {
    return object->y + object->height >= group->y + group->height || object->x + object->width >= group->x + group->width;
}

void gui_draw_spinbox(gui_base_t *base) {
    gui_draw_window(base);
    uint val = *((uint *)(base->data));
    uint padding = base->properties.padding;

    // General button preparation
    gui_base_t button = *base;
    gui_status_cast_t *button_status = (gui_status_cast_t *)&button.status;
    button_status->word = 0;
    button.status.visible = base->status.visible;
    button.status.enabled = base->status.enabled;
    button.width  = base->properties.horiz_vert ? GUI_BUTTON_MIN  : base->width - 2;
    button.height = base->properties.horiz_vert ? base->height - 2 : GUI_BUTTON_MIN;
    button.x = base->x + 1;
    button.y = base->y + 1;
    gui_base_t text = button;
    text.status.focused = base->status.focused;

    // Button substract preparation
    button.status.activated = base->status.substract;
    button.data = "-";
    if (gui_object_overflow_group(&button, base)) {
        return;
    }
    gui_draw_button(&button);

    // Number display
    text.x += base->properties.horiz_vert ? (padding + button.width) : 0;
    text.y += base->properties.horiz_vert ? 0 : (padding + button.height);
    text.width  = base->properties.horiz_vert ? base->width - (2 * (padding + 1 + button.width)) : base->width - 2;
    text.height = base->properties.horiz_vert ? base->height - 2 : base->height - (2 * (padding + 1 + button.height));
    if (gui_object_overflow_group(&text, base)) {
        return;
    }
    char buff[20];
    sprintf(buff,"%d", val);
    text.data = buff;
    gui_draw_text(&text);

    //Button add preparation
    button.x = base->properties.horiz_vert ? (text.x + padding + text.width) : text.x;
    button.y = base->properties.horiz_vert ? text.y : (text.y + padding + text.height);
    if (gui_object_overflow_group(&button, base)) {
        return;
    }
    button.data = "+";
    button.status.activated = base->status.add;
    gui_draw_button(&button);
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

gui_status_t gui_status_update(gui_object_t *object, gui_status_t status, bool set_clear) {
    gui_status_cast_t *c_status_old = (gui_status_cast_t *) &(object->base.status);
    gui_status_cast_t *c_status_new = (gui_status_cast_t *) &status;
    c_status_new->word = set_clear ? (c_status_old->word | c_status_new->word) : (c_status_old->word & (~c_status_new->word));
    return c_status_new->bits;
}

gui_object_t *gui_event(gui_status_t status, gui_object_t *origin) {
    //Update self
    gui_status_cast_t *c_status_new = (gui_status_cast_t *) &status;
    gui_status_cast_t *c_status_old = (gui_status_cast_t *) &origin->base.status;
    gui_status_cast_t c_status_changed;
    c_status_changed.word = c_status_new->word ^ c_status_old->word;
    // Redraw events

    if (c_status_new != c_status_old && ((c_status_changed.word & gui_status_redrawable.word) != 0)) {
        //Update only the redrawable events for drawing and restore
        gui_status_cast_t original;
        original.word =  c_status_old->word;
        c_status_old->word = (c_status_old->word & ~gui_status_redrawable.word) | (c_status_new->word & gui_status_redrawable.word);
        gui_ref_draw(origin);
        c_status_old->word = original.word;
    }

    //Check subscribers of the event
    uint subcribers_updates_cnt = 0;
    gui_status_cast_t subcribers_updates_status[GUI_EVENT_SUB_UPD_MAX];
    gui_object_t *subcribers_updates_destination[GUI_EVENT_SUB_UPD_MAX];
    for (uint cnt = 0; cnt < GUI_EVENT_HANDLING_MAX; cnt++) {
        gui_event_subscription_t *event = &event_subscriptions[cnt];
        gui_status_cast_t *c_status_sub = (gui_status_cast_t *) &event->status;
        gui_status_cast_t c_status_match;
        c_status_match.word = c_status_sub->word & c_status_changed.word;

        if (&origin->base == event->origin && c_status_match.word > 0) {
            gui_cb_on_status_t status_handle = event->destination->status_handle;
            if (origin == event->destination) {
                // need to update
            }
            
            gui_status_cast_t dest_status_old = *((gui_status_cast_t *)(&event->destination->base.status));
            if (status_handle == NULL || status_handle(status, &origin->base, event->destination)) {
                if (origin != event->destination) {
                    gui_ref_draw(event->destination);
                }
                // If it was a status change of the status due the even callback, trigger and event
                gui_status_cast_t dest_status_new = *((gui_status_cast_t *)(&event->destination->base.status));
                if (dest_status_old.word != dest_status_new.word && subcribers_updates_cnt < GUI_EVENT_SUB_UPD_MAX) {
                    event->destination->base.status = dest_status_old.bits;
                    subcribers_updates_status[subcribers_updates_cnt].word = dest_status_old.word ^ dest_status_new.word;
                    subcribers_updates_destination[subcribers_updates_cnt] = event->destination;
                    subcribers_updates_cnt++;
                }
            }
        }
    }

    // After events publications, update current updateable status
    if (c_status_new != c_status_old && ((c_status_changed.word & gui_status_updateable.word) != 0)) {
        //Update only the redrawable events
        c_status_old->word = (c_status_old->word & ~gui_status_updateable.word) | (c_status_new->word & gui_status_updateable.word);
    }

    // Consumable events like Data updates have to be reset after
    c_status_old->word &= ~gui_status_consumable.word;

    // Updates from subscriber events
    for (uint cnt = 0; cnt < subcribers_updates_cnt; cnt++) {
        gui_object_t *update_dest = subcribers_updates_destination[cnt]; 
        gui_status_cast_t update_status;
        update_status.word = ((gui_status_cast_t *)(&update_dest->base.status))->word ^ subcribers_updates_status[cnt].word;
        gui_event(update_status.bits, update_dest);
    }

    // Check focus change
    gui_object_t *next_focused = origin;
    if (status.go_next ^ status.go_previous) {
        if (status.go_next && (origin->next != NULL)) {
            status.go_next = 0;
            next_focused = origin->next;
        }
        if (status.go_previous && (origin->previous != NULL)) {
            status.go_previous = 0;
            next_focused = origin->previous;
        }
        if(next_focused != origin) {
            gui_unfocused(origin);
            gui_focused(next_focused);
        }
    }
    
    return next_focused;
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
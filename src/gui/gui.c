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
const char* gui_id_text    = "text";
const char* gui_id_label   = "label";
const char* gui_id_group   = "group";
const char* gui_id_spinbox = "spinbox";

// GUI events
const gui_status_t      gui_status_enabled      = { .enabled = 1 };
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

const char *gui_colors_str[] = { "border enabled", "fill unfocused", "main disabled", 
    "text enabled", "fill activated", "fill focused" };

// ---- Base object creation ----
gui_object_t gui_create_object(const graphic_ctx_t *ctx, uint x, uint y, uint width, uint height, const char* id, 
    gui_list_t *colors, gui_properties_t props, const void *data, gui_cb_draw_t draw_cb) {
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
                .visible = 1,
                .enabled = 1,
                .navigable = 1,
                .focused = 0
            },
            .properties = props,
            .data = data
        },
        .draw = draw_cb,
        .next = NULL,
        .previous = NULL
    };

    uint fill_group(gui_object_t *object, void *data) {
        gui_object_t **previous = (gui_object_t **) data;
        (*previous)->next = object;
        object->previous = *previous;
        *previous = object;
        return 1;
    }

    if (id == gui_id_group) { // We don't need a strcmp on known const keys
        gui_list_t *list = (gui_list_t *) data;
        gui_object_t **s_objects = (gui_object_t **)(list->elements);
        gui_object_t *objects = (gui_object_t *)(list->elements);
        gui_object_t *previous = props.shared ? s_objects[list->size - 1] : &objects[list->size - 1];
        
        gui_group_execute(&(obj.base), &previous, fill_group, NULL);
    }
    return obj;
}

// ----  Utility Functions  ----
void gui_get_start_position(gui_properties_t props, int ext_width, uint ext_height, uint in_width, uint in_height, uint *x, uint *y) {
    volatile uint width_space = ext_width > in_width ? ext_width - in_width : 0;
    volatile uint height_space = ext_height > in_height ? ext_height - in_height: 0;
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
        colors[4] : (base->status.focused && base->properties.focusable) ? colors[5] : colors[1];
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
    const char *scroll_text = text;
    uint8_t text_lines = 0;
    uint8_t text_max_per_line = 0;
    uint16_t curr_chars;
    const char *found;
    while ((found = strchr(scroll_text, '\n')) != NULL) {
        text_lines++;
        curr_chars = found - scroll_text;
        text_max_per_line = curr_chars > text_max_per_line ? curr_chars : text_max_per_line;
        scroll_text = found + 1;
    }
    curr_chars = strlen(scroll_text);
    if (curr_chars > 0) { text_lines++; }
    text_max_per_line = curr_chars > text_max_per_line ? curr_chars : text_max_per_line;

    uint text_width  = (text_max_per_line * GRAPHICS_FONT_SIZE) - 1;
    uint text_height = text_lines * GRAPHICS_FONT_SIZE - 1;
    uint x, y;
    gui_get_start_position(sub_base.properties, text_ctx.width, text_ctx.height, text_width, text_height, &x, &y);
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

typedef struct gui_draw_group_data {
   uint16_t inc_pos_x, inc_pos_y;
   uint8_t  padding;
   bool horiz_vert;
   bool enabled;
} gui_draw_group_data_t;

void gui_draw_group(gui_base_t *base) {
    uint draw_group(gui_object_t *object, void *data) {
        gui_draw_group_data_t *groupdata = (gui_draw_group_data_t *) data;
        gui_base_t *sub_base = &(object->base);
        sub_base->x = groupdata->inc_pos_x;
        sub_base->y = groupdata->inc_pos_y;
        gui_status_t sub_base_cached_status = sub_base->status;

        //Cache status
        if (!groupdata->enabled) {
            sub_base->status.enabled = false;
            sub_base->status.focused = false;
        }

        gui_ref_draw(object);
        
        //Restore draw status changes from parent
        sub_base->status = sub_base_cached_status;

        if (groupdata->horiz_vert) {
            groupdata->inc_pos_x += sub_base->width + groupdata->padding;
        } else {
            groupdata->inc_pos_y += sub_base->height + groupdata->padding;
        }

        return 1;
    }

    gui_draw_group_data_t group_data = {
        .inc_pos_x  = base->x,
        .inc_pos_y  = base->y,
        .padding    = base->properties.padding,
        .horiz_vert = base->properties.horiz_vert,
        .enabled    = base->status.enabled
    };

    if (base->properties.border) {
        gui_draw_window(base);
        group_data.inc_pos_x += 1;
        group_data.inc_pos_y += 1;
    }

    gui_group_execute(base, &group_data, draw_group, gui_object_overflow_group);
}

bool gui_object_overflow_group(gui_base_t *object, gui_base_t *group) {
    return object->y + object->height > group->y + group->height || object->x + object->width > group->x + group->width;
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
    text.properties.border = 0;
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
bool gui_event_subscribe(gui_status_t status, gui_base_t *origin, gui_object_t *destination, gui_cb_on_status_t status_cb) {
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
        sel_event->status = status;
        sel_event->origin = origin;
        sel_event->destination = destination;
        sel_event->status_handle = status_cb;
        return true;
    }
    return false;
}

uint gui_group_execute(gui_base_t *group, void *data, gui_cb_group_t group_cb, gui_cb_group_break_t break_cb) {
    if (group->id != gui_id_group) {
        return false;
    }
    gui_list_t *list = (gui_list_t *) group->data;
    bool shared = group->properties.shared;
    gui_object_t **s_objects = (gui_object_t **)(list->elements);
    gui_object_t *objects = (gui_object_t *)(list->elements);
    uint found = 0;
    for (uint8_t cnt = 0; cnt < list->size; cnt++) {
        gui_object_t *object = shared ? s_objects[cnt] : &objects[cnt];
        if (break_cb != NULL) {
            if (break_cb(&(object->base), group)) {
                return false;
            }
        }
        found += group_cb(object, data);
    }
    return found;
}

uint gui_event_unsubscribe(gui_base_t *origin, gui_object_t *destination) {
    uint unsubscribe_group(gui_object_t *object, void *data) {
        gui_object_t *_destination = (gui_object_t *)data;
        return gui_event_unsubscribe(&(object->base), _destination);
    }

    //Search stored status
    uint found = 0;
    for (uint cnt = 0; cnt < GUI_EVENT_HANDLING_MAX; cnt++) {
       gui_event_subscription_t *event = &event_subscriptions[cnt];
       gui_status_cast_t *status = (gui_status_cast_t *) &event->status;
       if (origin == event->origin && (destination == event->destination || destination == NULL)) {
            status->word = 0;
            event->origin = NULL;
            event->destination = NULL;
            event->status_handle = NULL;
            found++;
            if (destination != NULL) {
                return found;
            }
       } else if (origin->id == gui_id_group) {
            found += gui_group_execute(origin, destination, unsubscribe_group, NULL);
       }
    }
    return found;
}

gui_status_t gui_status_update(gui_object_t *object, gui_status_t status, bool set_clear) {
    gui_status_cast_t *c_status_old = (gui_status_cast_t *) &(object->base.status);
    gui_status_cast_t *c_status_new = (gui_status_cast_t *) &status;
    c_status_new->word = set_clear ? (c_status_old->word | c_status_new->word) : (c_status_old->word & (~c_status_new->word));
    return c_status_new->bits;
}

gui_object_t *gui_event(gui_status_t status, gui_object_t *origin) {
    if (origin == NULL) {
        return NULL;
    }
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
    bool propagate = true;
    for (uint cnt = 0; cnt < GUI_EVENT_HANDLING_MAX; cnt++) {
        gui_event_subscription_t *event = &event_subscriptions[cnt];
        gui_status_cast_t *c_status_sub = (gui_status_cast_t *) &event->status;
        gui_status_cast_t c_status_match;
        c_status_match.word = c_status_sub->word & c_status_changed.word;

        if (&origin->base == event->origin && c_status_match.word > 0) {
            gui_cb_on_status_t status_handle = event->status_handle;
            if (origin == event->destination) {
                // need to update
            }
            
            gui_status_cast_t dest_status_old = *((gui_status_cast_t *)(&event->destination->base.status));
            if (status_handle != NULL) {
                propagate &= status_handle(status, &origin->base, event->destination);
            }
            if (status_handle == NULL || propagate) {
                if (origin != event->destination && event->destination != NULL) {
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
            if (!propagate) {
                // The consumer of the event request no more propagation
                break;
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

    if (!propagate) {
        // The consumer of the event request no more events due to this event
        return origin;
    }

    // Updates from subscriber events
    for (uint cnt = 0; cnt < subcribers_updates_cnt; cnt++) {
        gui_object_t *update_dest = subcribers_updates_destination[cnt]; 
        gui_status_cast_t update_status;
        update_status.word = ((gui_status_cast_t *)(&update_dest->base.status))->word ^ subcribers_updates_status[cnt].word;
        gui_event(update_status.bits, update_dest);
    }

    // Check if it was a focus request and it's not focusable
    // WARNING: Request focus over a list of chained elements with at least one focusable
    if (status.focused && !origin->base.properties.focusable && !status.go_next & !status.go_previous) {
        status.go_next = 1;
    }

    // Check focus change
    gui_object_t *next_focused = origin;

    if (status.go_next ^ status.go_previous) {
        gui_object_t *test_focused = origin;
        for (uint cnt = 0; cnt < GUI_FOCUSABLE_SEARCH_MAX; cnt++) {
             if (status.go_next && (test_focused->next != NULL)) {
                test_focused = test_focused->next;
            }

            if (status.go_previous && (test_focused->previous != NULL)) {
                test_focused = test_focused->previous;
            }

            if (test_focused->base.properties.focusable) {
                status.go_next = 0;
                status.go_previous = 0;
                next_focused = test_focused;
                break;
            }
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
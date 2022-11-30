#ifndef _GUI_H
#define _GUI_H

#include "graphics.h"
#define GUI_BAR_100PERCENT     10000
#define GUI_EVENT_HANDLING_MAX 32
#define GUI_EVENT_SUB_UPD_MAX  16

enum gui_colors {
   gui_color_bg     = 0,
   gui_color_fg     = 1,
   gui_color_main   = 2,
   gui_color_second = 3
};

enum gui_orientation {
   gui_orientation_vertical   = 0,
   gui_orientation_horizontal = 1
};

enum gui_bool_op {
   gui_clear = 0,
   gui_set   = 1
};

enum gui_coordinate {
   gui_coord_height = 0,
   gui_coord_width  = 1
};

enum gui_alignment {
   gui_align_center       = 0,
   gui_align_center_up    = 1,
   gui_align_center_down  = 2,
   gui_align_left_center  = 3,
   gui_align_left_up      = 4,
   gui_align_left_down    = 5,
   gui_align_right_center = 6,
   gui_align_right_up     = 7,
   gui_align_right_down   = 8
};

typedef struct gui_list {
   void *elements;
   uint8_t size;
} gui_list_t;

typedef struct gui_status {
   uint16_t visible:1;
   uint16_t enabled:1;
   uint16_t focused:1;
   uint16_t activated:1;
   uint16_t data_changed:1;
   uint16_t go_previous:1;
   uint16_t go_next:1;
   uint16_t navigable:1;
   uint16_t add:1;
   uint16_t substract:1;
} gui_status_t;

typedef union gui_status_cast {
    gui_status_t  bits;
    uint16_t      word;
} gui_status_cast_t;

typedef struct gui_properties {
   uint16_t alignment:4;
   uint16_t horiz_vert:1;
   uint16_t padding:6;
   uint16_t focusable:1;
   uint16_t shared:1;
   uint16_t border:1;
} gui_properties_t;

typedef struct gui_base {
   const graphic_ctx_t *ctx;
   const char       *id;
   uint              x, y;
   uint              width, height;
   gui_list_t       *colors;
   gui_list_t       *event_subscribers;
   gui_status_t      status;
   gui_properties_t  properties;
   const void       *data;
} gui_base_t;

typedef struct gui_object gui_object_t ;

//gui_cb_group_t is a callback to be executed on each element on a group iteratively
typedef bool (*gui_cb_group_t)(gui_object_t *object, void *data);
typedef bool (*gui_cb_group_break_t)(gui_base_t *sub_base, gui_base_t *base);

//gui_cb_draw_t is a drawing callback
typedef void (*gui_cb_draw_t)(gui_base_t *data);

// gui_cb_on_status_t is the event callback handler from an origin object to a destination target
// status: the changing status
// origin: the originator of the event
// destination: the destination of the event
// result: (bool) request to propagate the event or not
typedef bool (*gui_cb_on_status_t)(gui_status_t status, gui_base_t *origin, gui_object_t *destination);
typedef void (*print_delegate_t) (const char * format, ...);
typedef void (*print_delegate_caller_t)(print_delegate_t printer);

typedef struct gui_object {
   gui_base_t         base;
   gui_cb_draw_t      draw;
   gui_cb_on_status_t status_handle;
   gui_object_t       *next, *previous;
} gui_object_t;

typedef struct gui_event_subscription {
   gui_status_t  status;
   gui_base_t   *origin;
   gui_object_t *destination;
} gui_event_subscription_t;

#define arraySize(list) (sizeof(list) / sizeof(list[0]))
#define initalizeGuiDynList(list, _size) { .elements = (void *)list, .size = _size }
#define initalizeGuiList(list) { .elements = (void *)list, .size = arraySize(list) }

// GUI Draw callbacks
void gui_draw_window(gui_base_t *base);
void gui_draw_focus(gui_base_t *base);
void gui_draw_text(gui_base_t *base);
void gui_draw_button(gui_base_t *base);
void gui_draw_slider(gui_base_t *base);
void gui_draw_label(gui_base_t *base);
void gui_draw_group(gui_base_t *base);
void gui_draw_spinbox(gui_base_t *base);

// GUI object draw def
#define gui_obj_draw(object)  object.draw(&(object.base))
#define gui_ref_draw(ref)     ref->draw(&(ref->base))

// GUI event and subscriber/unsubscriber
bool gui_event_subscribe(gui_status_t status, gui_base_t *origin, gui_object_t *destination, gui_cb_on_status_t status_cb);
bool gui_event_unsubscribe(gui_base_t *origin, gui_object_t *destination);
gui_object_t *gui_event(gui_status_t status, gui_object_t *origin);

// Event Status exports
extern const gui_status_t gui_status_enabled;
extern const gui_status_t gui_status_focused;
extern const gui_status_t gui_status_go_next;
extern const gui_status_t gui_status_go_previous;
extern const gui_status_t gui_status_activated;
extern const gui_status_t gui_status_data_changed;
extern const gui_status_t gui_status_add;
extern const gui_status_t gui_status_substract;

// Event triggers
gui_status_t gui_status_update(gui_object_t *object, gui_status_t status, bool set_clear);
#define gui_enable(object)         gui_event(gui_status_update(object, gui_status_enabled,      gui_set),   object)
#define gui_disable(object)        gui_event(gui_status_update(object, gui_status_enabled,      gui_clear), object)
#define gui_focused(object)        gui_event(gui_status_update(object, gui_status_focused,      gui_set),   object)
#define gui_unfocused(object)      gui_event(gui_status_update(object, gui_status_focused,      gui_clear), object)
#define gui_next_focus(object)     gui_event(gui_status_update(object, gui_status_go_next,      gui_set),   object)
#define gui_previous_focus(object) gui_event(gui_status_update(object, gui_status_go_previous,  gui_set),   object)
#define gui_activate(object)       gui_event(gui_status_update(object, gui_status_activated,    gui_set),   object)
#define gui_deactivate(object)     gui_event(gui_status_update(object, gui_status_activated,    gui_clear), object)
#define gui_update_data(object)    gui_event(gui_status_update(object, gui_status_data_changed, gui_set),   object)
#define gui_set_add(object)        gui_event(gui_status_update(object, gui_status_add,          gui_set),   object)
#define gui_set_sub(object)        gui_event(gui_status_update(object, gui_status_substract,    gui_set),   object)
#define gui_clear_add(object)      gui_event(gui_status_update(object, gui_status_add,          gui_clear), object)
#define gui_clear_sub(object)      gui_event(gui_status_update(object, gui_status_substract,    gui_clear), object)

// GUI Object creators
gui_object_t gui_create_object(const graphic_ctx_t *ctx, uint x, uint y, uint width, uint height, const char* id,
   gui_list_t *colors, gui_properties_t props, const uint8_t *data, gui_cb_draw_t draw_cb);

// GUI Ids
extern const char* gui_id_window;
extern const char* gui_id_button;
extern const char* gui_id_slider;
extern const char* gui_id_label;
extern const char* gui_id_group;
extern const char* gui_id_spinbox;

#define gui_create_window(ctx, x, y, width, height, colors, props) \
   gui_create_object(ctx, x, y, width, height, gui_id_window, colors, props, NULL, gui_draw_window)
#define gui_create_button(ctx, x, y, width, height, colors, props, text) \
   gui_create_object(ctx, x, y, width, height, gui_id_button, colors, props, (void *) text, gui_draw_button)
#define gui_create_slider(ctx, x, y, width, height, colors, props, number) \
   gui_create_object(ctx, x, y, width, height, gui_id_slider, colors, props, (void *) number, gui_draw_slider)
#define gui_create_label(ctx, x, y, width, height, colors,  props, print_fn) \
   gui_create_object(ctx, x, y, width, height, gui_id_label, colors, props, (void *) print_fn, gui_draw_label)
#define gui_create_group(ctx, x, y, width, height, colors,  props, list) \
   gui_create_object(ctx, x, y, width, height, gui_id_group, colors, props, (void *) list, gui_draw_group)
#define gui_create_spinbox(ctx, x, y, width, height, colors,  props, number) \
   gui_create_object(ctx, x, y, width, height, gui_id_spinbox, colors, props, (void *) number, gui_draw_spinbox)

// GUI Utilities
uint gui_sum(gui_list_t *group, gui_properties_t props, bool width_height);
bool gui_object_overflow_group(gui_base_t *object, gui_base_t *group);
bool gui_group_execute(gui_base_t *group, void *data, gui_cb_group_t group_cb, gui_cb_group_break_t break_cb);
#endif
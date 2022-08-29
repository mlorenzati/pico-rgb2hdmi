#ifndef _GUI_H
#define _GUI_H

#include "graphics.h"
#define GUI_BAR_100PERCENT     10000
#define GUI_EVENT_HANDLING_MAX 32

enum gui_colors {
   gui_color_bg     = 0,
   gui_color_fg     = 1,
   gui_color_main   = 2,
   gui_color_second = 3
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
   uint8_t visible:1;
   uint8_t enabled:1;
   uint8_t focused:1;
   uint8_t activated:1;
   uint8_t data_changed:1;   
} gui_status_t;

typedef union gui_status_cast {
    gui_status_t  bits;
    uint8_t       word;
} gui_status_cast_t;

typedef struct gui_properties {
   uint8_t alignment:4;
   uint8_t horiz_vert:1;
   uint8_t padding:6;
} gui_properties_t;

typedef struct gui_base {
   const graphic_ctx_t *ctx;
   uint              x, y;
   uint              width, height;
   gui_list_t       *colors;
   gui_list_t       *event_subscribers;
   gui_status_t      status;
   gui_properties_t  properties;
   const void       *data;
} gui_base_t;

typedef struct gui_object gui_object_t ;

typedef void (*gui_cb_draw_t)(gui_base_t *data);
typedef void (*gui_cb_on_status_t)(gui_status_t status, gui_base_t *origin, gui_object_t *destination);
typedef void (*print_delegate_t) (const char * format, ...);
typedef void (*print_delegate_caller_t)(print_delegate_t printer);

typedef struct gui_object {
   gui_base_t         base;
   gui_cb_draw_t      draw;
   gui_cb_on_status_t status_handle;
} gui_object_t;

typedef struct gui_event_subscription {
   gui_status_t  status;
   gui_base_t   *origin;
   gui_object_t *destination;
} gui_event_subscription_t;

#define initalizeGuiList(list) { .elements = (void *)list, .size = (sizeof(list) / sizeof(list[0])) }

// GUI Draw callbacks
void gui_draw_window(gui_base_t *base);
void gui_draw_focus(gui_base_t *base);
void gui_draw_text(gui_base_t *base);
void gui_draw_button(gui_base_t *base);
void gui_draw_slider(gui_base_t *base);
void gui_draw_label(gui_base_t *base);
void gui_draw_group(gui_base_t *base);

// GUI event and subscriber/unsubscriber
bool gui_event_subscribe(gui_status_t status, gui_base_t *origin, gui_object_t *destination);
bool gui_event_unsubscribe(gui_base_t *origin, gui_object_t *destination);
void gui_event(gui_status_t status, gui_object_t object, gui_status_t event);

// Event Handler callbacks
void gui_handle_button(gui_status_t status, gui_base_t *origin, gui_object_t *destination);
void gui_handle_text(gui_status_t status,   gui_base_t *origin, gui_object_t *destination);
void gui_handle_slider(gui_status_t status, gui_base_t *origin, gui_object_t *destination);
void gui_handle_label(gui_status_t status,  gui_base_t *origin, gui_object_t *destination);

// GUI Object creators
gui_object_t gui_create_object(const graphic_ctx_t *ctx, uint x, uint y, uint width, uint height, 
   gui_list_t *colors, gui_properties_t props, const uint8_t *data, gui_cb_draw_t draw_cb, gui_cb_on_status_t status_cv);

#define gui_create_window(ctx, x, y, width, height, colors, props) \
   gui_create_object(ctx, x, y, width, height, colors, props, NULL, gui_draw_window, NULL)
#define gui_create_button(ctx, x, y, width, height, colors, props, text) \
   gui_create_object(ctx, x, y, width, height, colors, props, (void *) text, gui_draw_button, gui_handle_button)
#define gui_create_slider(ctx, x, y, width, height, colors, props, number) \
   gui_create_object(ctx, x, y, width, height, colors, props, (void *) number, gui_draw_slider, gui_handle_slider)
#define gui_create_label(ctx, x, y, width, height, colors, props, print_fn) \
   gui_create_object(ctx, x, y, width, height, colors, props, (void *) print_fn, gui_draw_label, gui_handle_label)
#define gui_create_group(ctx, x, y, width, height, colors, props, list) \
   gui_create_object(ctx, x, y, width, height, colors, props, (void *) list, gui_draw_group, NULL)
#endif
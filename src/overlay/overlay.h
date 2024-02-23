
#ifndef _OVERLAY_H
#define _OVERLAY_H

#include "videoAdjust.h"

#define VIDEO_OVERLAY_GET_COMPUTED_WIDTH()       video_overlay.computed_width
#define VIDEO_OVERLAY_GET_COMPUTED_FRONT_PORCH() video_overlay.computed_front_porch
#define VIDEO_OVERLAY_GET_COMPUTED_OFFSET()      video_overlay.computed_offset

typedef struct video_overlay {
    io_rw_16         width;
    io_rw_16         height;
    io_rw_16         computed_front_porch;
    io_rw_16         computed_width;
    io_rw_16         computed_offset;
    io_rw_8          enabled:1;
    io_rw_8          left_right:1;
    io_rw_8          up_down:1;
} video_overlay_t;

extern video_overlay_t video_overlay;

inline io_rw_16 video_overlay_get_startx() {
    return video_overlay.left_right ? GET_VIDEO_PROPS().width - video_overlay.width : 0;
}

inline io_rw_16 video_overlay_get_starty() {
    return video_overlay.up_down ? GET_VIDEO_PROPS().height - video_overlay.height : 0;
}

void set_video_overlay(volatile signed int width, volatile signed int height, bool enabled);

static inline void video_overlay_scanline_prepare(unsigned int render_line_number) {
    unsigned int next_scanlineNumber = (render_line_number + 1);
    if (next_scanlineNumber >= GET_VIDEO_PROPS().height) {
        next_scanlineNumber = 0; 
    }

    if (video_overlay.enabled && 
        ((( video_overlay.up_down) && (next_scanlineNumber + video_overlay.height > GET_VIDEO_PROPS().height)) || 
         ((!video_overlay.up_down) && (next_scanlineNumber < video_overlay.height)))) {
        if (video_overlay.left_right) {
            video_overlay.computed_front_porch = get_video_prop_horizontal_front_porch();
            video_overlay.computed_offset = 0;
        } else {
            video_overlay.computed_front_porch = get_video_prop_horizontal_front_porch() + video_overlay.width;
            video_overlay.computed_offset = video_overlay.width;
        }
        video_overlay.computed_width = GET_VIDEO_PROPS().width - video_overlay.width;
    } else {
        video_overlay.computed_width = GET_VIDEO_PROPS().width;
        video_overlay.computed_front_porch = get_video_prop_horizontal_front_porch();
        video_overlay.computed_offset = 0;
    }
}

inline void video_overlay_enable(bool value) {
    video_overlay.enabled = value;
}

inline bool is_video_overlay_enabled() {
    return video_overlay.enabled;
}

#endif
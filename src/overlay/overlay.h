
#ifndef _OVERLAY_H
#define _OVERLAY_H

#include "videoAdjust.h"

#define VIDEO_OVERLAY_GET_COMPUTED_WIDTH()       video_overlay.computed_width
#define VIDEO_OVERLAY_GET_COMPUTED_FRONT_PORCH() video_overlay.computed_front_porch
#define VIDEO_OVERLAY_GET_COMPUTED_OFFSET()      video_overlay.computed_offset

typedef struct video_overlay {
    video_props_t   *video_props;
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

void set_video_overlay(volatile signed int width, volatile signed int height, bool enabled);

inline void video_overlay_scanline_prepare(unsigned int render_line_number) {
    unsigned int next_scanlineNumber = (render_line_number + 1) % video_overlay.video_props->height;

    if (video_overlay.enabled && 
        ((( video_overlay.up_down) && (next_scanlineNumber + video_overlay.height > video_overlay.video_props->height)) || 
         ((!video_overlay.up_down) && (next_scanlineNumber < video_overlay.height)))) {
        if (video_overlay.left_right) {
            video_overlay.computed_front_porch = video_overlay.video_props->horizontal_front_porch;
            video_overlay.computed_offset = 0;
        } else {
            video_overlay.computed_front_porch = video_overlay.video_props->horizontal_front_porch + video_overlay.width;
            video_overlay.computed_offset = video_overlay.width;
        }
        video_overlay.computed_width = video_overlay.video_props->width - video_overlay.width;
    } else {
        video_overlay.computed_width = video_overlay.video_props->width;
        video_overlay.computed_front_porch = video_overlay.video_props->horizontal_front_porch;
        video_overlay.computed_offset = 0;
    }
}

#endif
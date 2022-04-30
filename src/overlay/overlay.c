#include "overlay.h"

#define abs(x) x > 0 ? x : -x

video_overlay_t video_overlay;

void set_video_overlay(volatile signed int width, volatile signed int height, bool enabled) {
    video_overlay.video_props = &(GET_VIDEO_PROPS());
    video_overlay.left_right = width > 0;
    video_overlay.up_down = height > 0;
    video_overlay.width =  abs(width)  > video_overlay.video_props->width  ? video_overlay.video_props->width  : abs(width);
    video_overlay.height = abs(height) > video_overlay.video_props->height ? video_overlay.video_props->height : abs(height);
    video_overlay.enabled = enabled;

    video_overlay.computed_width = 0;
    video_overlay.computed_offset = 0;
    video_overlay.computed_front_porch = video_overlay.video_props->horizontal_front_porch;
}
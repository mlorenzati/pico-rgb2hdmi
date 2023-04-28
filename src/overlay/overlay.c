#include "overlay.h"

#define abs(x) x > 0 ? x : -x

video_overlay_t video_overlay;

void set_video_overlay(volatile signed int width, volatile signed int height, bool enabled) {
    video_overlay.left_right = width > 0;
    video_overlay.up_down = height > 0;
    video_overlay.width =  abs(width)  > GET_VIDEO_PROPS().width  ? GET_VIDEO_PROPS().width  : abs(width);
    video_overlay.height = abs(height) > GET_VIDEO_PROPS().height ? GET_VIDEO_PROPS().height : abs(height);
    video_overlay.enabled = enabled;

    video_overlay.computed_width = 0;
    video_overlay.computed_offset = 0;
    video_overlay.computed_front_porch = get_video_prop_horizontal_front_porch();
}
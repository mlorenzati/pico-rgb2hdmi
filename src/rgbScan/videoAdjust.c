#include "videoAdjust.h"

video_props_t video_props;

void set_video_props(
    io_rw_16    vertical_front_porch,
    io_rw_16    vertical_back_porch,
    io_rw_16    horizontal_front_porch,
    io_rw_16    horizontal_back_porch,
    io_rw_16    width,
    io_rw_16    height,
    io_rw_8     refresh_rate,
    void        *video_buffer
    ) {
    video_props.vertical_front_porch = vertical_front_porch;
    video_props.vertical_back_porch = vertical_back_porch;
    video_props.horizontal_front_porch = horizontal_front_porch;
    video_props.horizontal_back_porch = horizontal_back_porch;
    video_props.width = width;
    video_props.height = height;
    video_props.refresh_rate = refresh_rate;
    update_sampling_rate();
    video_props.video_buffer = video_buffer;
}

void update_sampling_rate() {
    video_props.sampling_rate = 
        (video_props.horizontal_front_porch + video_props.width + video_props.horizontal_back_porch) * 
        (video_props.vertical_front_porch + video_props.height + video_props.vertical_back_porch) * 
        video_props.refresh_rate;
}
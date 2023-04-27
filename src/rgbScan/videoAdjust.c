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
    bool        symbols_per_word,
    void        *video_buffer
    ) {
    video_props.vertical_front_porch = vertical_front_porch;
    video_props.vertical_back_porch = vertical_back_porch;
    video_props.horizontal_front_porch = horizontal_front_porch;
    video_props.horizontal_back_porch =  horizontal_back_porch;
    video_props.width = width;
    video_props.height = height;
    video_props.refresh_rate = refresh_rate;
    video_props.symbols_per_word = symbols_per_word;
    video_props.video_buffer = video_buffer;
    update_sampling_rate();
}

io_rw_16 get_video_prop_horizontal_front_porch() {
    return (video_props.horizontal_front_porch >> video_props.symbols_per_word) + 
        ((video_props.symbols_per_word * video_props.horizontal_front_porch) >> (video_props.symbols_per_word * 3));
}

io_rw_16 get_video_prop_horizontal_back_porch() {
    return (video_props.horizontal_back_porch  >> video_props.symbols_per_word) -
        ((video_props.symbols_per_word * video_props.horizontal_back_porch) >> (video_props.symbols_per_word * 4));
}

void update_sampling_rate() {
    io_rw_16 horizontal_front_porch = get_video_prop_horizontal_front_porch();
    io_rw_16 horizontal_back_porch  = get_video_prop_horizontal_back_porch();
    video_props.sampling_rate = 
        (horizontal_front_porch + video_props.width + horizontal_back_porch) * 
        (video_props.vertical_front_porch + video_props.height + video_props.vertical_back_porch) * 
        video_props.refresh_rate;
}
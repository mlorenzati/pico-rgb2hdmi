#ifndef _VIDEO_ADJUST_H
#define _VIDEO_ADJUST_H

#include "hardware/address_mapped.h"

#define VIDEO_FINE_TUNE_MAX 100000

typedef struct video_props {
    io_rw_16    vertical_front_porch;
    io_rw_16    vertical_back_porch;
    io_rw_16    horizontal_front_porch;
    io_rw_16    horizontal_back_porch;
    io_rw_16    width;
    io_rw_16    height;
    io_rw_8     refresh_rate;
    io_rw_32    sampling_rate;
    void        *video_buffer;
    bool        symbols_per_word;
    signed int  fine_tune;
} video_props_t;

extern video_props_t video_props;

#define GET_VIDEO_PROPS()   video_props

void set_video_props(
    io_rw_16    vertical_front_porch,
    io_rw_16    vertical_back_porch,
    io_rw_16    horizontal_front_porch,
    io_rw_16    horizontal_back_porch,
    io_rw_16    width,
    io_rw_16    height,
    io_rw_8     refresh_rate,
    signed int  fine_tune,
    bool        symbols_per_word,
    void        *video_buffer
    );

void update_sampling_rate();
io_rw_16 get_video_prop_horizontal_front_porch();
io_rw_16 get_video_prop_horizontal_back_porch();
#endif
#ifndef COLOR_H
#define COLOR_H
#include "pico.h"

typedef enum {
	rgb_8_332  = 1,
    rgb_16_565 = 2,
    rgb_24_888 = 3
} color_bppx;

typedef enum {
    color_part_red,
    color_part_green,
    color_part_blue,
    color_part_all
} color_part;


uint8_t bppx_to_int(color_bppx bppx, color_part part);
uint    bppx_get_color(color_bppx bppx, void *buffer);
void    bppx_put_color(color_bppx bppx, void *buffer, uint color);
void    bppx_split_color(color_bppx bppx, uint color, uint *red, uint *green, uint *blue, bool normalized);
uint    bppx_merge_color(color_bppx bppx, uint8_t red, uint8_t green, uint8_t blue, bool normalized);
#endif
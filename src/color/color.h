#ifndef COLOR_H
#define COLOR_H
#include "pico.h"

typedef enum {
	rgb_8_332  = 1,
    rgb_16_565 = 2,
    rgb_24_888 = 3
} color_bppx;

uint8_t bppx_to_int(color_bppx bppx);
uint    bppx_get_color(color_bppx bppx, void *buffer);
void    bppx_put_color(color_bppx bppx, void *buffer, uint color);
#endif
#include "color.h"

uint8_t bppx_to_int(color_bppx bppx, color_part part) {

    uint8_t b_red, b_green, b_blue;
    switch (bppx) {
        case rgb_8_332:  
            b_red = 3; b_green = 3; b_blue = 2;
            break;
        case rgb_16_565:
            b_red = 5; b_green = 6; b_blue = 5;
            break;
        case rgb_24_888:
            b_red = 8; b_green = 8; b_blue = 8;
            break;
        default: return 0;
    }

    switch (part)
    {
        case color_part_red:   return b_red;
        case color_part_green: return b_green;
        case color_part_blue:  return b_blue;
        default: return b_red + b_green + b_blue;
    }
}

uint bppx_get_color(color_bppx bppx, void *buffer) {
    switch (bppx) {
        case rgb_8_332:  return *((uint8_t  *)buffer) & 0xFF;
        case rgb_16_565: return *((uint16_t *)buffer) & 0xFFFF;
        case rgb_24_888: return *((uint32_t *)buffer) & 0xFFFFFF;
        default: return 0;
    }
}

void bppx_put_color(color_bppx bppx, void *buffer, uint color) {
    switch (bppx) {
        case rgb_8_332:  *((uint8_t  *)buffer) = color & 0xFF;     break;
        case rgb_16_565: *((uint16_t *)buffer) = color & 0xFFFF;   break;
        case rgb_24_888: *((uint32_t *)buffer) = color & 0xFFFFFF; break;
        default: break;
    }
}

void bppx_split_color(color_bppx bppx, uint color, uint *red, uint *green, uint *blue) {
    uint8_t b_red, b_green, b_blue;
    switch (bppx) {
        case rgb_8_332:  
            b_red = 3; b_green = 3; b_blue = 2;
            break;
        case rgb_16_565:
            b_red = 5; b_green = 6; b_blue = 5;
            break;
        case rgb_24_888:
            b_red = 8; b_green = 8; b_blue = 8;
            break;
        default: return;
    }
    *red   = (color >> (b_green + b_blue)) & ((1 << b_red)   - 1);
    *green = (color >> b_blue)             & ((1 << b_green) - 1);
    *blue  = (color)                       & ((1 << b_blue)  - 1);
}

uint bppx_merge_color(color_bppx bppx, uint8_t red, uint8_t green, uint8_t blue) {
    uint8_t b_red = 0;
    uint8_t b_green = 0;
    uint8_t b_blue = 0;
    switch (bppx) {
        case rgb_8_332:  
            b_red = 3; b_green = 3; b_blue = 2;
            break;
        case rgb_16_565:
            b_red = 5; b_green = 6; b_blue = 5;
            break;
        case rgb_24_888:
            b_red = 8; b_green = 8; b_blue = 8;
            break;
        default: return 0;
    }
    return ((red&((1<<b_red) - 1))<<(b_green+b_blue))|((green&((1<<b_green) - 1))<<b_blue)|(blue&((1<<b_blue) - 1));
}
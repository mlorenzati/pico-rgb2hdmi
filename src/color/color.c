#include "color.h"

uint8_t bppx_to_int(color_bppx bppx) {
    switch (bppx) {
        case rgb_8_332:  return 8;
        case rgb_16_565: return 16;
        case rgb_24_888: return 24;
        default: return 0;
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
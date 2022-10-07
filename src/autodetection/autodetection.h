#ifndef AUTODETECTION_H
#define AUTODETECTION_H

#include "pico.h"

#define DETECTION_BUFFER_SHIFT   5
#define DETECTION_BUFFER_SIZE    (1<<DETECTION_BUFFER_SHIFT)
#define DETECTION_LINES_SHIFT    1
#define DETECTION_LINES_SIZE     (1<<DETECTION_LINES_SHIFT)

#define DETECTION_INIT_LINE      10

#if DVI_SYMBOLS_PER_WORD == 2
    #define PIXEL_GET_RED(x)        x>>11
    #define PIXEL_GET_GREEN(x)      x>>5
    #define PIXEL_GET_BLUE(x)       x
    #define PIXEL_SET_RGB(r,g,b)    r<<11|g<<5|b
    #define DETECTION_HSYNC_FRONT   60
    extern uint16_t  detection_buffer[DETECTION_BUFFER_SIZE];
    extern uint16_t  detection_average[DETECTION_LINES_SIZE];
    inline uint16_t get_detection_average() {
        return detection_average[(DETECTION_LINES_SIZE>>1)];
    } 
#else
    #define PIXEL_GET_RED(x)        x>>6
    #define PIXEL_GET_GREEN(x)      x>>3
    #define PIXEL_GET_BLUE(x)       x
    #define PIXEL_SET_RGB(r,g,b)    r<<6|g<<3|b
    #define DETECTION_HSYNC_FRONT   120
    extern uint8_t   detection_buffer[DETECTION_BUFFER_SIZE];
    extern uint8_t   detection_average[DETECTION_LINES_SIZE];
    inline uint8_t get_detection_average() {
        return detection_average[(DETECTION_LINES_SIZE>>1)];
    }
#endif

extern unsigned int detection_first_video_line;
extern bool detection_ready;

void __not_in_flash_func(detectionLineTriggered)(unsigned int line_number);
inline unsigned int getInitialVideoLine() {
    return detection_first_video_line + (DETECTION_LINES_SIZE>>1);
}
inline bool is_detection_ready() {
    return detection_ready;
}

#endif
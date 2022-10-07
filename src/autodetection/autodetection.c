#include "autodetection.h"
#include "pico/stdlib.h"
#include "wm8213Afe.h"

#if DVI_SYMBOLS_PER_WORD == 2
    uint16_t  detection_buffer[DETECTION_BUFFER_SIZE];
    uint16_t  detection_average[DETECTION_LINES_SIZE];
#else
    uint8_t   detection_buffer[DETECTION_BUFFER_SIZE];
    uint8_t   detection_average[DETECTION_LINES_SIZE];
#endif

unsigned int detection_first_video_line = DETECTION_INIT_LINE;
bool detection_ready = false;

void __not_in_flash_func(detectionLineTriggered)(unsigned int line_number) {
    static unsigned int previous_line_number;
    if (line_number < detection_first_video_line) {
        return;
    }
    if (line_number < detection_first_video_line + DETECTION_LINES_SIZE) {
        if (wm8213_afe_capture_run(DETECTION_HSYNC_FRONT >> 1, (uintptr_t)detection_buffer, DETECTION_BUFFER_SIZE)) {
        //Nothing is done here so far
        }

        uint16_t red = 0;
        uint16_t green = 0;
        uint16_t blue = 0;
        for (uint8_t cnt = 0; cnt < DETECTION_BUFFER_SIZE; cnt++) {
            //Add R G B separatedly then divide
            red   += PIXEL_GET_RED(detection_buffer[cnt]);
            green += PIXEL_GET_GREEN(detection_buffer[cnt]);
            blue  += PIXEL_GET_BLUE(detection_buffer[cnt]);
        }
        red   = red   >> DETECTION_BUFFER_SHIFT;
        green = green >> DETECTION_BUFFER_SHIFT;
        blue  = blue  >> DETECTION_BUFFER_SHIFT;
        detection_average[previous_line_number - detection_first_video_line] = PIXEL_SET_RGB(red, green, blue);
        previous_line_number = line_number;
    } else if (line_number == detection_first_video_line + DETECTION_LINES_SIZE) {
        if (!detection_ready) {
            if (detection_average[(DETECTION_LINES_SIZE>>1)-1] > 0) {
                detection_first_video_line--;
            } else if(detection_average[(DETECTION_LINES_SIZE>>1)] == 0) {
                detection_first_video_line++;
            } else {
                detection_ready = true;
            }
        } else {
            if (detection_average[(DETECTION_LINES_SIZE>>1)] > 0) {
                //TBD
            }
        }
    }
}
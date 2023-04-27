/**
 *  Initial tests of the rgbScan api
 */
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "wm8213Afe.h"

//System configuration includes
#include "version.h"
#include "common_configs.h"

void core1_main();

uint16_t test_buf[32];

// A simple test the AFE (Analog Front End)
int main() {
    stdio_init_all();
    printf("AFE initial test \n");
    wm8213_afe_init(&afec_cfg);
    wm8213_afe_capture_update_bppx(rgb_16_565, false);
    if (wm8213_afe_start(2000000) > 0) {
         printf("AFE initialize failed \n");
    } else {
         printf("AFE initialize succeded \n");
    }

    for (int cnt = 0; cnt < 32; cnt++) {
        test_buf[cnt]=0xFFFF;
    }
    
    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    multicore_launch_core1(core1_main);

    printf("%s version - AFE Test %s started!\n", PROJECT_NAME, PROJECT_VER);

    while (true) {
        gpio_put(LED_PIN, 1);
        sleep_ms(500);
        gpio_put(LED_PIN, 0);
        sleep_ms(500);
        wm8213_afe_capture_run(1, (uintptr_t)test_buf, 32);
        wm8213_afe_capture_wait();
        printf("RED\tGREEN\tBLUE\n");
        for (int cnt = 0; cnt < 32; cnt++) {
            int b = test_buf[cnt] & 0x1F; 
            int g = (test_buf[cnt]>>5) & 0x3F;
            int r = (test_buf[cnt]>>11) & 0x1F;
            printf("%f\t%f\t%f\n",(float)r*3.3/31, (float)g*3.3/63, (float)b*3.3/31);
        }
        printf("\n");
    }
}

void core1_main() {
    while (true) {
        sleep_ms(1000);
    }
}
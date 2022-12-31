/**
 *  Initial tests of the rgbScan api
 */
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "wm8xxxAfe.h"
#include "color.h"

//System configuration includes
#include "version.h"
#include "common_configs.h"

void core1_main();

uint16_t test_buf[32];

// A simple test the AFE (Analog Front End)
int main() {
    stdio_init_all();
    printf("AFE initial test \n");
    if (wm8xxx_afe_setup(&afec_cfg, 5000000) > 0) {
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
        wm8xxx_afe_capture_run(1, (uintptr_t)test_buf, 32);
        wm8xxx_afe_capture_wait();
        printf("RED\tGREEN\tBLUE\n");
        for (int cnt = 0; cnt < 32; cnt++) {
            uint red   = 0;
            uint green = 0;
            uint blue  = 0;
            bppx_split_color(afec_cfg.bppx, test_buf[cnt], &red, &green, &blue, true);
           
            printf("%d\t%d\t%d\n",red, green, blue);
        }
        printf("\n");
    }
}

void core1_main() {
    while (true) {
        sleep_ms(1000);
    }
}
/**
 *  Initial tests of the rgbScan api
 */
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "wm8213Afe.h"
#include "common_configs.h"


void core1_main();

// A simple test the AFE (Analog Front End)
int main() {
    stdio_init_all();
    printf("AFE initial test \n");
    if (wm8213_afe_setup(&afec_cfg) > 0) {
         printf("AFE initialize failed \n");
    } else {
         printf("AFE initialize succeded \n");
    }
    
    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    multicore_launch_core1(core1_main);

    while (true) {
        gpio_put(LED_PIN, 1);
        sleep_ms(1000);
        gpio_put(LED_PIN, 0);
        sleep_ms(1000);
    }
}

void core1_main() {
    while (true) {
        sleep_ms(1000);
    }
}
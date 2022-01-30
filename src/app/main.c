/**
 */
#include <stdio.h>
#include "pico/stdlib.h"
#include "common_configs.h"

int main() {
    stdio_init_all();

    sleep_ms(1000);
    printf("Pico rgb 2 vga\n");

    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    while (true) {
        gpio_put(LED_PIN, 1);
        printf("Led On\n");
        sleep_ms(250);
        gpio_put(LED_PIN, 0);
        printf("Led Off\n");
        sleep_ms(250);
    }
}

/**
 *  Initial tests of the rgbScan api
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "storage.h"
#include "common_configs.h"

const uint LED_PIN = PICO_DEFAULT_LED_PIN;
bool blink = true;

#define TEST_BUFFER_SIZE 255
uint8_t buffer[TEST_BUFFER_SIZE];
const void *settings_in_flash;

int main() {
    //stdio_init_all();

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    uint flash_capacity = 0;

    //Prepare random data
    for (int i = 0; i < TEST_BUFFER_SIZE; ++i) {
        buffer[i] = rand() >> 16;
    }

    while (true) {
        flash_capacity = storage_get_flash_capacity();
        
        printf("Storage capacity of %d bytes\n", flash_capacity);

        int res = storage_initialize(buffer, &settings_in_flash, 255);
        if (res > 0) {
            printf("Storage inialization failed\n");
        } else if (res < 0) {
            printf("Storage inialization success, first time\n");
        } else {
            printf("Storage inialization success\n");
        }
        
        //Compare settings
        res = strncmp(buffer, settings_in_flash, TEST_BUFFER_SIZE);
        if (res != 0) {
            printf("Storage mirror copied wrong initial data\n");
        } else {
            printf("Storage mirror inialization success\n"); 
        }

        sleep_ms(2000);
    }
}
/**
 *  Initial tests of the rgbScan api
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "security.h"

//We are including sha1 header for the sake of the test
#include "sha1.h"

const uint LED_PIN = PICO_DEFAULT_LED_PIN;
bool blink = true;

int main() {
    stdio_init_all();

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);


    char message[SECURITY_MESSAGE_SIZE + 1];
    static uint8_t key[SHA1HashSize] = {0};
    static uint8_t bad_key[SHA1HashSize] = {0};
    SHA1Context sha;
    SHA1Reset(&sha);
    sprintf(message, "%s%s", SECURITY_SALT, security_get_uid());
    SHA1Input(&sha, message, SECURITY_MESSAGE_SIZE);
    SHA1Result(&sha, key);

    int token = -1;
    memcpy(bad_key, key, SHA1HashSize);
    bad_key[SHA1HashSize>>1] = 0xFF;
    int res;
    while (true) {
        printf("Security module test\n");
        const char *unique_id = security_get_uid();
        printf("The unique Id of this device is %s\n", unique_id);

        res = security_key_is_valid(key, token);
        printf("The good securiy key is %s\n", res > 0 ? "invalid" : "valid");
        if (res <= 0) {
            token = res;
        }

        res = security_key_is_valid(bad_key, token);
        printf("The bad securiy key is %s\n", res > 0 ? "invalid" : "valid");        

        sleep_ms(2000);
    }
    __builtin_unreachable();
}
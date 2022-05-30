/**
 *  Initial tests of the rgbScan api
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "security.h"

//System configuration includes
#include "version.h"

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

    char key_str[41];
    char bad_key_str[41];
    
    security_hexa_2_str(key, key_str, 20);
    security_hexa_2_str(bad_key, bad_key_str, 20);

    const char *hexa_test_str = "0123456789ABCDEF";
    uint8_t hexa_test[8];  
    
    while (true) {
        printf("Security module Test %s - version %s started!\n", PROJECT_NAME, PROJECT_VER);
        const char *unique_id = security_get_uid();
        printf("The unique Id of this device is %s\n", unique_id);

        int res = security_str_2_hexa(hexa_test_str, hexa_test, 16);
        if (res) {
            printf("security_str_2_hexa returned error %d\n", res);
        } else {
            const uint8_t hexa_test_valid[] = { 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF };
            res = strncmp(hexa_test, (const char*) hexa_test_valid, 8);
            printf("security_str_2_hexa converted %s %s\n", hexa_test_str, res == 0 ? "succesfully" : "wrong"); 
        }

        res = security_key_is_valid(key, token);
        printf("The good security key (%s) is %s\n", key_str, res > 0 ? "invalid" : "valid");
        if (res <= 0) {
            token = res;
        }

        res = security_key_is_valid(bad_key, token);
        printf("The bad  security key (%s) is %s\n", bad_key_str, res > 0 ? "invalid" : "valid");

        printf("The stored firmware security signing is %s @ %x\n", security_get_signing_str(), security_get_signing());      

        sleep_ms(2000);
    }
    __builtin_unreachable();
}
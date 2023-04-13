#include "security.h"
#include "sha1.h"
#include <stdio.h>
#include "string.h"

const char* security_get_uid() {
    static char uid[SECURITY_UID_SIZE + 1] = {0};
    
    if (uid[0] == 0) {
        pico_unique_board_id_t board_id;
        pico_get_unique_board_id(&board_id);
        security_hexa_2_str(board_id.id, uid, PICO_UNIQUE_BOARD_ID_SIZE_BYTES);
    }
    
    return uid;
}

void security_hexa_2_str(const uint8_t *in, char *out, uint8_t size) {
    char *hex_print=out;

    for (int i = 0; i < size; ++i) {
        sprintf(hex_print, "%02x", in[i]);
        hex_print+=2;
    }
    *hex_print = 0;
}

char security_get_nibble(char nibble) {
    if (nibble <= '9') {
        nibble -= '0';
    } else if (nibble >= 'A' && nibble <= 'F') {
        nibble = nibble - 'A' + 0xA;
    } else if (nibble >= 'a' && nibble <= 'f') {
        nibble = nibble - 'a' + 0xA;
    } else { nibble = 0; }
    return nibble;
}

int security_str_2_hexa(const char *in, uint8_t *out, uint8_t size) {
    if (size % 2 > 0) { // The size has to be 2N
        return 1;
    }
    for (int i = 0; i < size; i+=2) {
        char nibble1 = in[i];
        char nibble2 = in[i+1];
        if (nibble1 == 0 || nibble2 == 0) { //we encountered prior end of size an end of line
            return 2;
        }
        *out = security_get_nibble(nibble1) << 4 | security_get_nibble(nibble2);
        out++;
    }
    return 0;
}

int security_key_is_valid(const uint8_t *key, int token) {
    static int last_token = 0;
    static uint8_t digest[SHA1HashSize] = {0};
    static const uint8_t *last_key = NULL;

    if (token == last_token || token > 0) {
        return 1;
    }

    if (key == NULL) {
        return 2;
    }

    if (last_key != key) {
        uint8_t message[SECURITY_MESSAGE_SIZE+ 1];
        SHA1Context sha;
        SHA1Reset(&sha);
        sprintf((char *)message, "%s%s", SECURITY_SALT, security_get_uid());
        SHA1Input(&sha, message, SECURITY_MESSAGE_SIZE);
        SHA1Result(&sha, digest);
        last_key = key;
    }

    if (strncmp((const char *)key, (const char *)digest, SHA1HashSize) != 0) {
        return 3;
    }

    last_token = token;

    token = token - 1 > 0 ? 0 : token - 1;

    return token;
}

extern char __flash_binary_end;
const uint8_t *security_get_signing() {
    return (const uint8_t *) &__flash_binary_end;
}

const char *security_get_signing_str() {
    static char security_signature[2*SHA1HashSize + 1];
    security_hexa_2_str(security_get_signing(), security_signature, SHA1HashSize);
    return security_signature;
}
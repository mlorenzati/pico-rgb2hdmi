#include "security.h"
#include "sha1.h"
#include "pico/unique_id.h"
#include <stdio.h>

const char* security_get_uid() {
    static char uid[2*PICO_UNIQUE_BOARD_ID_SIZE_BYTES + 1] = {0};
    
    if (uid[0] == 0) {
        pico_unique_board_id_t board_id;
        pico_get_unique_board_id(&board_id);
        char *uid_print=uid;

        for (int i = 0; i < PICO_UNIQUE_BOARD_ID_SIZE_BYTES; ++i) {
            sprintf(uid_print, "%02x", board_id.id[i]);
            uid_print+=2;
        }
        uid[2*PICO_UNIQUE_BOARD_ID_SIZE_BYTES] = 0;
    }
    
    return uid;
}

int security_key_is_valid(const char *key, int token) {
    static int last_token = 0;
    static uint8_t digest[SHA1HashSize] = {0};

    if (token == last_token || token > 0) {
        return 1;
    }

    if (digest[0] != key[0]) {
        char message[SECURITY_MESSAGE_SIZE + 1];
        SHA1Context sha;
        SHA1Reset(&sha);
        sprintf(message, "%s%s", SECURITY_SALT, security_get_uid());
        SHA1Input(&sha, message, SECURITY_MESSAGE_SIZE);
        SHA1Result(&sha, digest);
    }

    if (strncmp(key, digest, SHA1HashSize) != 0) {
        return 2;
    }

    last_token = token;

    token = token - 1 > 0 ? 0 : token - 1;

    return token;
}
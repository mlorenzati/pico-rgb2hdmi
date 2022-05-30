
#ifndef _SECURITY_H
#define _SECURITY_H

#include "hardware/address_mapped.h"
#include "pico/unique_id.h"

#define SECURITY_SALT           "Z&7@g45a"
#define SECURITY_SALT_SIZE      8
#define SECURITY_UID_SIZE       2*PICO_UNIQUE_BOARD_ID_SIZE_BYTES
#define SECURITY_MESSAGE_SIZE   SECURITY_SALT_SIZE + SECURITY_UID_SIZE

const char*    security_get_uid();
int            security_key_is_valid(const char *key, int token);
void           security_hexa_2_str(const uint8_t *in, char *out, uint8_t size);
int            security_str_2_hexa(const char *in, uint8_t *out, uint8_t size);
const uint8_t *security_get_signing();
const char    *security_get_signing_str();
#endif
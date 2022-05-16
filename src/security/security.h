
#ifndef _SECURITY_H
#define _SECURITY_H

#define SECURITY_SALT "Z&7@g45a"
#define SECURITY_MESSAGE_SIZE 24

const char* security_get_uid();
int         security_key_is_valid(const char *key, int token);

#endif
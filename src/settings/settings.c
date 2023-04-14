#include <string.h>
#include "settings.h"
#include "storage.h"

const settings_t *flash_settings;
const settings_t *p_factory_settings;

settings_t ram_settings;

int settings_initialize(const settings_t *p_fact_sett) {
    p_factory_settings = p_fact_sett;

    //Initializes storage, copies to flash first time and brings settings from flash
    if (storage_initialize(p_factory_settings, (const void **)&flash_settings, sizeof(settings_t), false) > 0) {
		return 1;
	}
    
    //Copies all flash settings to ram
    memcpy(&ram_settings, flash_settings, sizeof(settings_t));

    // The flash setting is an old one, default to factory except the license
    if (flash_settings->eof_canary > 0) {
        memcpy((uint8_t *)(&ram_settings) + SECURITY_SHA_SIZE, (uint8_t *)(p_factory_settings) + SECURITY_SHA_SIZE, sizeof(settings_t) - SECURITY_SHA_SIZE);
    }

    #ifdef INITIAL_LICENSE
        //Set the license on RAM, if the license on flash is different from the one generated, it will flash it
		const char *security_key_str = INITIAL_LICENSE;
		security_str_2_hexa(security_key_str, ram_settings.security_key, SECURITY_SHA_SIZE * 2);
        if (strncmp((const char *)ram_settings.security_key, (const char *)flash_settings->security_key, SECURITY_SHA_SIZE) != 0) {
            settings_factory();
        }
	#endif

    return 0;
}

int settings_update() {
    return storage_update(&ram_settings);
}

int settings_factory() {
    // All data is lost except the security key which is the beginning of the setting area
    // Set factory settings after the key area
    memcpy((uint8_t *)(&ram_settings) + SECURITY_SHA_SIZE, (uint8_t *)(p_factory_settings) + SECURITY_SHA_SIZE, sizeof(settings_t) - SECURITY_SHA_SIZE);
    return storage_update(&ram_settings);
}
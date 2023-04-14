#include <string.h>
#include "settings.h"
#include "storage.h"

const settings_t *flash_settings;
const settings_t *p_factory_settings;

settings_t ram_settings;

int settings_initialize(const settings_t *p_fact_sett) {
    const settings_t *initial_settings;
    bool force_storage = false;
    p_factory_settings = p_fact_sett;
    #ifdef INITIAL_LICENSE
		const char *security_key_str = INITIAL_LICENSE;
        memcpy(&ram_settings, p_factory_settings, sizeof(settings_t));
		security_str_2_hexa(security_key_str, ram_settings.security_key, SECURITY_SHA_SIZE * 2);
        initial_settings = &ram_settings;
		force_storage = true;
	#else 
        initial_settings = p_factory_settings;
	#endif

    if (storage_initialize(initial_settings, (const void **)&flash_settings, sizeof(settings_t), force_storage) > 0) {
		return 1;
	}

    return 0;
}

int settings_update() {
    return storage_update(&ram_settings);
}

int settings_factory() {
    memcpy(&ram_settings, p_factory_settings, sizeof(settings_t));
    return storage_update(&ram_settings);
}
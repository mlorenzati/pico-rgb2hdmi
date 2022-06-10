#include "storage.h"

#include "pico/stdlib.h"
#include "hardware/flash.h"
#include "hardware/sync.h"
#include <string.h>

#define STORAGE_CMD_DUMMY_BYTES 1
#define STORAGE_CMD_DATA_BYTES 3
#define STORAGE_CMD_TOTAL_BYTES (STORAGE_CMD_DUMMY_BYTES + STORAGE_CMD_DATA_BYTES)

size_t   global_storage_erase_size = 0;
size_t   global_storage_write_size = 0;
uint32_t global_storage_offset = 0;

uint storage_get_flash_capacity() {
    uint8_t txbuf[STORAGE_CMD_TOTAL_BYTES] = {0x9f};
    uint8_t rxbuf[STORAGE_CMD_TOTAL_BYTES] = {0};
    flash_do_cmd(txbuf, rxbuf, STORAGE_CMD_TOTAL_BYTES);

    return 1 << rxbuf[3];
}

size_t get_size_in_blocks(size_t in_size, size_t block_size) {
    return ((in_size / block_size) + (in_size % block_size > 0)) * block_size;
}

void storage_flash(uint32_t offset, size_t size, const void *settings) {
    uint8_t buffer[FLASH_PAGE_SIZE];

    // Set Canary to off
    buffer[0] = 0; 
    for (int idx = 0; idx < size; idx+=FLASH_PAGE_SIZE) {

        memcpy(idx == 0 ? buffer + 1 : buffer, idx == 0 ? settings : settings + idx - 1, idx == 0 ? FLASH_PAGE_SIZE - 1 : FLASH_PAGE_SIZE);
        
        //Disable interrupts prior saving
        uint32_t status = save_and_disable_interrupts();
        flash_range_program(offset+idx, buffer, FLASH_PAGE_SIZE);
        restore_interrupts(status);
    }
} 

int storage_initialize(const void *initial_settings, const void **updated_settings, size_t size, bool force) {
    // Size is +1 to host the canary byte
    global_storage_erase_size = get_size_in_blocks(size + 1, FLASH_SECTOR_SIZE);
    global_storage_write_size = get_size_in_blocks(size + 1, FLASH_PAGE_SIZE);
    uint32_t capacity = storage_get_flash_capacity();
    if (capacity == 0 || capacity >= (1<<30)) {
        // Stop if we get an erroneous capacity from the QSPI Flash
        return 1;
    }
    
    global_storage_offset = capacity - global_storage_erase_size;
    const uint8_t *global_storage_memory_offset = (const uint8_t *) (XIP_BASE + global_storage_offset);
    const uint8_t canary = global_storage_memory_offset[0];
    *updated_settings = &(global_storage_memory_offset[1]);
    
     // First byte is the canary, if unitialized requires to be flashed
    if (canary != 0 || force) {
        // Flash data
        storage_update(initial_settings);

        return -1;
    }

    return 0;
}

int storage_update(const void *settings) {
    // Erase full range, disable IRQs to avoid XIP errors
    uint32_t status = save_and_disable_interrupts();
    flash_range_erase(global_storage_offset, global_storage_erase_size);
   
    // Update pages
    storage_flash(global_storage_offset, global_storage_write_size, settings);
    restore_interrupts(status);
    return 0;
}
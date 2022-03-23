#include "wm8213Afe.h"
#include "hardware/gpio.h"

static uint afe_cs = -1;
static spi_inst_t *afe_spi;
static bool spi_write_only_mode = true;

int wm8213_enable_cs(bool val) {
    asm volatile("nop \n nop \n nop");
    gpio_put(afe_cs, val);
    asm volatile("nop \n nop \n nop");
}

void wm8213_trigger_cs() {
    asm volatile("nop \n nop \n nop");
    gpio_put(afe_cs, true);
    sleep_us(1);
    gpio_put(afe_cs, false);
    asm volatile("nop \n nop \n nop");
}

int wm8213_afe_write(const uint8_t address, uint8_t data) {
    uint8_t buffer[2] = { WM8213_ADDR_WRITE(address), data };
    size_t read_len = spi_write_blocking(afe_spi, buffer, 2);
    wm8213_trigger_cs();
    return read_len != 2;
}

int wm8213_afe_read(uint8_t address, uint8_t *data) {
    uint8_t buffer[2] = { WM8213_ADDR_READ(address), 0};
    size_t read_len = spi_write_blocking(afe_spi, buffer, 2);
    if (read_len != 2) {
        return 1;
    }
    wm8213_trigger_cs();

    read_len = spi_read_blocking(afe_spi, 0, data, 1);
    return read_len != 1;
}

int wm8213_afe_setup(const wm8213_afe_config_t* config)
{
    // Configure SPI head and the baudrate
    afe_spi = config->spi;
    spi_init(afe_spi, config->baudrate);

    // Configure pins
    gpio_set_function(config->pins.sck, GPIO_FUNC_SPI);
    gpio_set_function(config->pins.sdi, GPIO_FUNC_SPI);
    spi_write_only_mode = (config->pins.sdo >= NUM_BANK0_GPIOS) || config->setups.setup2.opd;
    if (!spi_write_only_mode) {
        gpio_set_function(config->pins.sdo, GPIO_FUNC_SPI);
    }
    // Configure and disable CS
    afe_cs = config->pins.cs;
    gpio_init(afe_cs);
    wm8213_enable_cs(false);
    gpio_set_dir(afe_cs, GPIO_OUT);

    // Reset the AFE
    if (wm8213_afe_write(WM8213_REG_SWRESET, 0) > 0) {
        return 1;
    }

    // Send each setup
    uint8_t setup_regs[6] = { WM8213_REG_SETUP1, WM8213_REG_SETUP2, WM8213_REG_SETUP3, WM8213_REG_SETUP4, WM8213_REG_SETUP5 ,WM8213_REG_SETUP6 };
    uint8_t *setup_vals = (uint8_t *) &(config->setups.setup1);
    //No verification is possible if opd is in true o there is no SDO pin connected
    assert(config->verify_retries == 0 || !spi_write_only_mode);
    for (char cnt=0; cnt < 6; cnt++) {
        if (wm8213_afe_write(setup_regs[cnt], setup_vals[cnt]) > 0) {
            return 2;
        }
        //Check data
        uint8_t readData;
        for (char retries = 0; retries < config->verify_retries; retries++) {
            if (wm8213_afe_read(setup_regs[cnt], &readData) > 0) {
                return 3;
            }
            if (readData == setup_vals[cnt]) {
                break;
            }
        }
        if ((config->verify_retries > 0) && (readData != setup_vals[cnt])) {
            return 4;
        }
    }

    return 0;
}
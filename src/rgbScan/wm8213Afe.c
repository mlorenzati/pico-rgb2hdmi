#include "wm8213Afe.h"
#include "hardware/gpio.h"
#include "wm8213Afe.pio.h"
#include "hardware/structs/bus_ctrl.h"

//Local registers
static uint        afe_cs = -1;
static spi_inst_t *afe_spi;
static bool        spi_write_only_mode = true;
uint16_t           dummy_dma_read;

//Global Afe Capture vars
wm8213_afe_capture_t wm8213_afe_capture_global;
bool capture_global_configured = false;

// AFE SPI Related
void wm8213_enable_cs(bool val) {
    asm volatile("nop \n nop \n nop");
    gpio_put(afe_cs, val);
    asm volatile("nop \n nop \n nop");
}

void wm8213_trigger_cs() {
    asm volatile("nop \n nop \n nop");
    gpio_put(afe_cs, true);
    busy_wait_us(1);
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

int wm8213_afe_spi_setup(const wm8213_afe_config_t* config) {
    // Configure SPI head and the baudrate
    afe_spi = config->spi;
    spi_init(afe_spi, config->baudrate);

    // Configure pins
    gpio_set_function(config->pins_spi.sck, GPIO_FUNC_SPI);
    gpio_set_function(config->pins_spi.sdi, GPIO_FUNC_SPI);
    spi_write_only_mode = (config->pins_spi.sdo >= NUM_BANK0_GPIOS) || config->setups.setup2.opd;
    if (!spi_write_only_mode) {
        gpio_set_function(config->pins_spi.sdo, GPIO_FUNC_SPI);
    }
    // Configure and disable CS
    afe_cs = config->pins_spi.cs;
    gpio_init(afe_cs);
    wm8213_enable_cs(false);
    gpio_set_dir(afe_cs, GPIO_OUT);

    // Reset the AFE
    if (wm8213_afe_write(WM8213_REG_SWRESET, 0) > 0) {
        return 1;
    }

    // Send each setup
    const uint8_t setup_regs[WM8213_REG_SETUP_TOTAL] = { WM8213_REG_SETUP1, WM8213_REG_SETUP2, WM8213_REG_SETUP3, WM8213_REG_SETUP4, WM8213_REG_SETUP5 ,WM8213_REG_SETUP6,
        WM8213_REG_OFFSET_DAC_RED, WM8213_REG_OFFSET_DAC_GRN, WM8213_REG_OFFSET_DAC_BLU, WM8213_REG_PGA_GAIN_LSB_RED, WM8213_REG_PGA_GAIN_MSB_RED,
        WM8213_REG_PGA_GAIN_LSB_GRN, WM8213_REG_PGA_GAIN_MSB_GRN, WM8213_REG_PGA_GAIN_LSB_BLU, WM8213_REG_PGA_GAIN_MSB_BLU };
    uint8_t *setup_vals = (uint8_t *) &(config->setups.setup1);
    //No verification is possible if opd is in true o there is no SDO pin connected
    assert(config->verify_retries == 0 || !spi_write_only_mode);
    for (unsigned char cnt=0; cnt < WM8213_REG_SETUP_TOTAL; cnt++) {
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

// AFE Pio Capture Related
uint wm8213_afe_capture_setup() {
    if (!capture_global_configured) {
        return 1;
    }
    pio_sm_set_enabled(wm8213_afe_capture_global.config.pio, wm8213_afe_capture_global.config.sm_afe, false);
    
    const pio_program_t *program = NULL;
    uint8_t op_bits = 0;
    uint    op_pins = 0;
    switch (wm8213_afe_capture_global.bppx) {
        case rgb_8_332:
            program = (wm8213_afe_capture_global.sampling_rate > AFE_SAMPLING_LIMIT) ? &afe_capture_332_inverted_program : &afe_capture_332_program; 
            op_bits = 3;
            op_pins = wm8213_afe_capture_global.config.pin_base_afe_op + 3;
            break;
        case rgb_16_565:
            program = (wm8213_afe_capture_global.sampling_rate > AFE_SAMPLING_LIMIT) ? &afe_capture_565_inverted_program : &afe_capture_565_program; 
            op_bits = 6;
            op_pins = wm8213_afe_capture_global.config.pin_base_afe_op;
            break;
        case rgb_24_888: //Not supported yet
        default:
            return 6;
    }
    
    if (wm8213_afe_capture_global.pio_offset != 0) {
        pio_sm_set_enabled(wm8213_afe_capture_global.config.pio, wm8213_afe_capture_global.config.sm_afe, false);
        pio_remove_program(wm8213_afe_capture_global.config.pio, program, wm8213_afe_capture_global.pio_offset);
        pio_sm_restart(wm8213_afe_capture_global.config.pio, wm8213_afe_capture_global.config.sm_afe);
    }

    wm8213_afe_capture_global.pio_offset = pio_add_program(wm8213_afe_capture_global.config.pio, program);
    afe_capture_program_init(wm8213_afe_capture_global.config.pio, wm8213_afe_capture_global.config.sm_afe, wm8213_afe_capture_global.pio_offset, wm8213_afe_capture_global.sampling_rate, op_pins, wm8213_afe_capture_global.config.pin_base_afe_ctrl, op_bits);
    
    // Give DMA R/W priority over the Bus
    //bus_ctrl_hw->priority = BUSCTRL_BUS_PRIORITY_DMA_W_BITS | BUSCTRL_BUS_PRIORITY_DMA_R_BITS;
    pio_sm_set_enabled(wm8213_afe_capture_global.config.pio, wm8213_afe_capture_global.config.sm_afe, true);
    return 0;
}

uint wm8213_afe_capture_update_sampling_rate(uint sampling_rate) {
    wm8213_afe_capture_global.sampling_rate = sampling_rate;
    return wm8213_afe_capture_setup();
}

uint wm8213_afe_capture_update_bppx(color_bppx bppx, bool commit) {
    wm8213_afe_capture_global.bppx = bppx;
    if (commit) {
        return wm8213_afe_capture_setup();
    }
    return 0;
}

// AFE DMA related
void afe_dma_prepare(PIO pio, uint sm) {
    wm8213_afe_capture_global.capture_dma = dma_claim_unused_channel(true);
    wm8213_afe_capture_global.front_porch_dma = dma_claim_unused_channel(true);

    dma_channel_config front_porch_dma_channel_cfg = dma_channel_get_default_config(wm8213_afe_capture_global.front_porch_dma);
    channel_config_set_transfer_data_size(&front_porch_dma_channel_cfg, wm8213_afe_capture_global.bppx == rgb_8_332 ? DMA_SIZE_8 : DMA_SIZE_16);   //Transfer 8/16bits words that are shifted by pio
    channel_config_set_dreq(&front_porch_dma_channel_cfg, pio_get_dreq(pio, sm, false)); // Pace transfers based on PIO samples availabilty
    channel_config_set_read_increment(&front_porch_dma_channel_cfg, false);
    channel_config_set_write_increment(&front_porch_dma_channel_cfg, false);
    channel_config_set_chain_to(&front_porch_dma_channel_cfg, wm8213_afe_capture_global.capture_dma);

    dma_channel_configure(wm8213_afe_capture_global.front_porch_dma,
        &front_porch_dma_channel_cfg,
        &dummy_dma_read,// Destination: dummy
        &pio->rxf[sm],  // Source
        0,              // Size: will be set later
        false
    );

    dma_channel_config afe_dma_channel_cfg = dma_channel_get_default_config(wm8213_afe_capture_global.capture_dma);
    channel_config_set_transfer_data_size(&afe_dma_channel_cfg, wm8213_afe_capture_global.bppx == rgb_8_332 ? DMA_SIZE_8 : DMA_SIZE_16);   //Transfer 8/16bits words that are shifted by pio
    channel_config_set_dreq(&afe_dma_channel_cfg, pio_get_dreq(pio, sm, false)); // Pace transfers based on PIO samples availabilty
    channel_config_set_read_increment(&afe_dma_channel_cfg, false);
    channel_config_set_write_increment(&afe_dma_channel_cfg, true);
    
    dma_channel_configure(wm8213_afe_capture_global.capture_dma,
        &afe_dma_channel_cfg,
        NULL,           // Destination: will be set later
        &pio->rxf[sm],  // Source
        0,              // Size: will be set later
        false
    );
}

//AFE Main calls
// AFE init gets the initial configuration from from and allow later modification prior executing
void wm8213_afe_init(const wm8213_afe_config_t* config) {
    if (config != NULL) {
        wm8213_afe_capture_global.config = *config;
        capture_global_configured = true;
    }
}   
// AFE setup executes the configuration from config or local and setups AFE and DMA (can be null config)
int wm8213_afe_start(uint sampling_rate)
{
    if (!capture_global_configured) {
        return 5;
    }

    wm8213_afe_config_t *config = &wm8213_afe_capture_global.config;
    int res = wm8213_afe_spi_setup(config);
    if (res > 0) { return res; }

    wm8213_afe_capture_global.sampling_rate = sampling_rate;

    res = wm8213_afe_capture_setup();
    if (res > 0) { return res; }

    afe_dma_prepare(config->pio, config->sm_afe);

    return res;
}

void wm8213_afe_capture_wait() {
    dma_channel_wait_for_finish_blocking(wm8213_afe_capture_global.capture_dma);
}

void wm8213_afe_capture_stop() {
    pio_sm_set_enabled(wm8213_afe_capture_global.config.pio, wm8213_afe_capture_global.config.sm_afe, false);
}

uint wm8213_afe_update_gain(uint16_t red, uint16_t green, uint16_t blue, bool commit) {
    if (red   > WM8213_GAIN_MAX) { red   = WM8213_GAIN_MAX; }
    if (green > WM8213_GAIN_MAX) { green = WM8213_GAIN_MAX; }
    if (blue  > WM8213_GAIN_MAX) { blue  = WM8213_GAIN_MAX; }
    wm8213_afe_pga_gain_rgb_t new_gain = {
        .red = {
            .lsb =  red & 0x01,
            .msb = (red >> 1) & 0xFF
        },
        .green = {
            .lsb =  green & 0x01,
            .msb = (green >> 1) & 0xFF
        },
        .blue = {
            .lsb =  blue & 0x01,
            .msb = (blue >> 1) & 0xFF
        }
    };
    wm8213_afe_capture_global.config.setups.pga_gain = new_gain;
    if (commit) {
        return wm8213_afe_spi_setup(&wm8213_afe_capture_global.config);
    } else {
        return 0;
    }
}

uint16_t wm8213_afe_get_gain(color_part part) {
    wm8213_afe_pga_gain_rgb_t *gain = &wm8213_afe_capture_global.config.setups.pga_gain;
    uint16_t red   = ((uint16_t) (gain->red.msb   << 1)) | (gain->red.lsb   & 0x1);
    uint16_t green = ((uint16_t) (gain->green.msb << 1)) | (gain->green.lsb & 0x1);
    uint16_t blue =  ((uint16_t) (gain->blue.msb  << 1)) | (gain->blue.lsb  & 0x1);

    switch (part) {
        case color_part_red:   return red;
        case color_part_green: return green;
        case color_part_blue:  return blue;
        default: return (red + green + blue) / 3;
    }
}

uint wm8213_afe_update_offset(uint8_t red, uint8_t green, uint8_t blue, bool commit) {
    wm8213_afe_offset_dac_t new_offset = {
        .red = red,
        .green = green,
        .blue = blue
    };
    wm8213_afe_capture_global.config.setups.offset_dac = new_offset;
    if (commit) {
        return wm8213_afe_spi_setup(&wm8213_afe_capture_global.config);
    } else {
        return 0;
    }
}

uint8_t wm8213_afe_get_offset(color_part part) {
    wm8213_afe_offset_dac_t *offset = &wm8213_afe_capture_global.config.setups.offset_dac;
    uint8_t red   = offset->red;
    uint8_t green = offset->green;
    uint8_t blue =  offset->blue;

     switch (part) {
        case color_part_red:   return red;
        case color_part_green: return green;
        case color_part_blue:  return blue;
        default: return ((uint16_t)red + (uint16_t)green + (uint16_t)blue) / 3;
    }
}

uint wm8213_afe_update_negative_offset(uint8_t value, bool commit) {
   if (value > WM8213_NEG_OFFSET_MAX) { value = WM8213_NEG_OFFSET_MAX; }
    wm8213_afe_capture_global.config.setups.setup3.rlc_dac = value;
    if (commit) {
        return wm8213_afe_spi_setup(&wm8213_afe_capture_global.config);
    } else {
        return 0;
    }
}

uint8_t wm8213_afe_get_negative_offset() {
    return wm8213_afe_capture_global.config.setups.setup3.rlc_dac & WM8213_NEG_OFFSET_BITS;
}
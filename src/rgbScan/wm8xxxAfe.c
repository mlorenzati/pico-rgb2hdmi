#include "wm8xxxAfe.h"
#include "hardware/gpio.h"
#include "wm8xxxAfe.pio.h"
#include "hardware/structs/bus_ctrl.h"

// Local registers
static uint        afe_cs = -1;
static spi_inst_t *afe_spi;
static bool        spi_write_only_mode = true;
uint16_t           dummy_dma_read;

// Global Afe Capture vars
wm8xxx_afe_capture_t wm8xxx_afe_capture_global;

// Constants
const char *afe_family_str[family_max] = { "unknown", "wm819x", "wm821x"};
const uint8_t wm821x_setup_regs[WM821X_REG_SETUP_TOTAL] = { WM8XXX_REG_SETUP1, WM8XXX_REG_SETUP2, WM8XXX_REG_SETUP3, WM8XXX_REG_SETUP4, WM821X_REG_SETUP5 ,WM821X_REG_SETUP6,
    WM8XXX_REG_OFFSET_DAC_RED, WM8XXX_REG_OFFSET_DAC_GRN, WM8XXX_REG_OFFSET_DAC_BLU, WM821X_REG_PGA_GAIN_LSB_RED, WM8XXX_REG_PGA_GAIN_MSB_RED,
    WM821X_REG_PGA_GAIN_LSB_GRN, WM8XXX_REG_PGA_GAIN_MSB_GRN, WM821X_REG_PGA_GAIN_LSB_BLU, WM8XXX_REG_PGA_GAIN_MSB_BLU };

const uint8_t wm819x_setup_regs[WM819X_REG_SETUP_TOTAL] = { WM8XXX_REG_SETUP1, WM8XXX_REG_SETUP2, WM8XXX_REG_SETUP3, WM8XXX_REG_SETUP4, WM819X_REG_SETUP5 ,WM819X_REG_SETUP6,
    WM8XXX_REG_OFFSET_DAC_RED, WM8XXX_REG_OFFSET_DAC_GRN, WM8XXX_REG_OFFSET_DAC_BLU, WM8XXX_REG_PGA_GAIN_MSB_RED, WM8XXX_REG_PGA_GAIN_MSB_GRN, WM8XXX_REG_PGA_GAIN_MSB_BLU };

// AFE SPI Related
void wm8xxx_enable_cs(bool val) {
    asm volatile("nop \n nop \n nop");
    gpio_put(afe_cs, val);
    asm volatile("nop \n nop \n nop");
}

void wm8xxx_trigger_cs() {
    asm volatile("nop \n nop \n nop");
    gpio_put(afe_cs, true);
    busy_wait_us(1);
    gpio_put(afe_cs, false);
    asm volatile("nop \n nop \n nop");
}

int wm8xxx_afe_write(const uint8_t address, uint8_t data) {
    uint8_t buffer[2] = { WM8XXX_ADDR_WRITE(address), data };
    size_t read_len = spi_write_blocking(afe_spi, buffer, 2);
    wm8xxx_trigger_cs();
    return read_len != 2;
}

int wm8xxx_afe_read(uint8_t address, uint8_t *data) {
    uint8_t buffer[2] = { WM8XXX_ADDR_READ(address), 0};
    size_t read_len = spi_write_blocking(afe_spi, buffer, 2);
    if (read_len != 2) {
        return 1;
    }
    wm8xxx_trigger_cs();

    read_len = spi_read_blocking(afe_spi, 0, data, 1);
    return read_len != 1;
}

int wm8xxx_afe_spi_setup(const wm8xxx_afe_config_t* config) {
    // Store config ptr in global
    if (config != NULL) {
        wm8xxx_afe_capture_global.config = *config;
    }

    // Configure SPI head and the baudrate
    afe_spi = wm8xxx_afe_capture_global.config.spi;
    spi_init(afe_spi, wm8xxx_afe_capture_global.config.baudrate);

    // Configure pins
    gpio_set_function(wm8xxx_afe_capture_global.config.pins_spi.sck, GPIO_FUNC_SPI);
    gpio_set_function(wm8xxx_afe_capture_global.config.pins_spi.sdi, GPIO_FUNC_SPI);
    spi_write_only_mode = (wm8xxx_afe_capture_global.config.pins_spi.sdo >= NUM_BANK0_GPIOS) || wm8xxx_afe_capture_global.config.setups.wm821x.setup2.opd;
    if (!spi_write_only_mode) {
        gpio_set_function(wm8xxx_afe_capture_global.config.pins_spi.sdo, GPIO_FUNC_SPI);
    }
    // Configure and disable CS
    afe_cs = wm8xxx_afe_capture_global.config.pins_spi.cs;
    gpio_init(afe_cs);
    wm8xxx_enable_cs(false);
    gpio_set_dir(afe_cs, GPIO_OUT);

    // Reset the AFE
    if (wm8xxx_afe_write(WM8XXX_REG_SWRESET, 0) > 0) {
        return 1;
    }

    // Check AFE Family (wm821x reports default value 0x0c and wm819x 0x00 on the PGA Gain register)
    uint8_t readData;
    if (wm8xxx_afe_read(WM8XXX_REG_PGA_GAIN_MSB_RED, &readData) > 0) {
        return 5;
    }
    
    uint8_t *setup_vals = NULL;
    uint8_t afe_total_reg = 0;
    const uint8_t *setup_regs = NULL;
    switch (readData) {
        case 0x00: 
            wm8xxx_afe_capture_global.config.setups.afe_family = family_wm819x;
            setup_regs = wm819x_setup_regs;
            setup_vals = (uint8_t *) &(wm8xxx_afe_capture_global.config.setups.wm819x.setup1);
            afe_total_reg = WM819X_REG_SETUP_TOTAL;
            break;
        case 0x0c: 
            wm8xxx_afe_capture_global.config.setups.afe_family = family_wm821x;
            setup_regs = wm821x_setup_regs;
            setup_vals = (uint8_t *) &(wm8xxx_afe_capture_global.config.setups.wm821x.setup1);
            afe_total_reg = WM821X_REG_SETUP_TOTAL;
            break;
        default:
            // Unknown response for the AFE in the selected register
            wm8xxx_afe_capture_global.config.setups.afe_family = family_unknown;
            return 6; 
    }

    //No verification is possible if opd is in true o there is no SDO pin connected
    assert(wm8xxx_afe_capture_global.config.verify_retries == 0 || !spi_write_only_mode);
    for (unsigned char cnt=0; cnt < afe_total_reg; cnt++) {
        if (wm8xxx_afe_write(setup_regs[cnt], setup_vals[cnt]) > 0) {
            return 2;
        }
        //Check data
        for (char retries = 0; retries < wm8xxx_afe_capture_global.config.verify_retries; retries++) {
            if (wm8xxx_afe_read(setup_regs[cnt], &readData) > 0) {
                return 3;
            }
            if (readData == setup_vals[cnt]) {
                break;
            }
        }
        if ((wm8xxx_afe_capture_global.config.verify_retries > 0) && (readData != setup_vals[cnt])) {
            return 4;
        }
    }

    return 0;
}

// AFE Pio Capture Related
void wm8xxx_afe_capture_setup_from_global() {
    const pio_program_t *program = NULL;
    uint8_t op_bits = 0;
    uint    op_pins = 0;
    switch (wm8xxx_afe_capture_global.bppx) {
        case rgb_8_332:
            program = (wm8xxx_afe_capture_global.sampling_rate > wm821x_SAMPLING_LIMIT) ? &afe_capture_332_inverted_program : &afe_capture_332_program; 
            op_bits = 3;
            op_pins = wm8xxx_afe_capture_global.op_pins + 3;
            break;
        case rgb_16_565:
            program = (wm8xxx_afe_capture_global.sampling_rate > wm821x_SAMPLING_LIMIT) ? &afe_capture_565_inverted_program : &afe_capture_565_program; 
            op_bits = 6;
            op_pins = wm8xxx_afe_capture_global.op_pins;
            break;
        case rgb_24_888: //Not supported yet
        default:
            assert(false);
            break;
    }
    if (wm8xxx_afe_capture_global.pio_offset != 0) {
        pio_sm_set_enabled(wm8xxx_afe_capture_global.pio, wm8xxx_afe_capture_global.sm, false);
        pio_remove_program(wm8xxx_afe_capture_global.pio, program, wm8xxx_afe_capture_global.pio_offset);
        pio_sm_restart(wm8xxx_afe_capture_global.pio, wm8xxx_afe_capture_global.sm);
    }

    wm8xxx_afe_capture_global.pio_offset = pio_add_program(wm8xxx_afe_capture_global.pio, program);
    afe_capture_program_init(wm8xxx_afe_capture_global.pio, wm8xxx_afe_capture_global.sm, wm8xxx_afe_capture_global.pio_offset, wm8xxx_afe_capture_global.sampling_rate, op_pins, wm8xxx_afe_capture_global.control_pins, op_bits);
    
    // Give DMA R/W priority over the Bus
    //bus_ctrl_hw->priority = BUSCTRL_BUS_PRIORITY_DMA_W_BITS | BUSCTRL_BUS_PRIORITY_DMA_R_BITS;
}
void wm8xxx_afe_capture_setup(PIO pio, uint sm, uint sampling_rate, color_bppx bppx, uint op_pins, uint control_pins) {
    //Setup OP and sample ports on PIO
    wm8xxx_afe_capture_global.pio = pio;
    wm8xxx_afe_capture_global.sm =  sm;
    wm8xxx_afe_capture_global.sampling_rate = sampling_rate;
    wm8xxx_afe_capture_global.bppx = bppx;
    wm8xxx_afe_capture_global.op_pins = op_pins;
    wm8xxx_afe_capture_global.control_pins = control_pins;
    
    wm8xxx_afe_capture_setup_from_global();
}

void wm8xxx_afe_capture_update_sampling_rate(uint sampling_rate) {
    wm8xxx_afe_capture_global.sampling_rate = sampling_rate;
    wm8xxx_afe_capture_setup_from_global();
}

void wm8xxx_afe_capture_update_bppx(color_bppx bppx) {
    wm8xxx_afe_capture_global.bppx = bppx;
    wm8xxx_afe_capture_setup_from_global();
}

// AFE DMA related
void afe_dma_prepare(PIO pio, uint sm) {
    wm8xxx_afe_capture_global.capture_dma = dma_claim_unused_channel(true);
    wm8xxx_afe_capture_global.front_porch_dma = dma_claim_unused_channel(true);

    dma_channel_config front_porch_dma_channel_cfg = dma_channel_get_default_config(wm8xxx_afe_capture_global.front_porch_dma);
    channel_config_set_transfer_data_size(&front_porch_dma_channel_cfg, wm8xxx_afe_capture_global.bppx == rgb_8_332 ? DMA_SIZE_8 : DMA_SIZE_16);   //Transfer 8/16bits words that are shifted by pio
    channel_config_set_dreq(&front_porch_dma_channel_cfg, pio_get_dreq(pio, sm, false)); // Pace transfers based on PIO samples availabilty
    channel_config_set_read_increment(&front_porch_dma_channel_cfg, false);
    channel_config_set_write_increment(&front_porch_dma_channel_cfg, false);
    channel_config_set_chain_to(&front_porch_dma_channel_cfg, wm8xxx_afe_capture_global.capture_dma);

    dma_channel_configure(wm8xxx_afe_capture_global.front_porch_dma,
        &front_porch_dma_channel_cfg,
        &dummy_dma_read,// Destination: dummy
        &pio->rxf[sm],  // Source
        0,              // Size: will be set later
        false
    );

    dma_channel_config afe_dma_channel_cfg = dma_channel_get_default_config(wm8xxx_afe_capture_global.capture_dma);
    channel_config_set_transfer_data_size(&afe_dma_channel_cfg, wm8xxx_afe_capture_global.bppx == rgb_8_332 ? DMA_SIZE_8 : DMA_SIZE_16);   //Transfer 8/16bits words that are shifted by pio
    channel_config_set_dreq(&afe_dma_channel_cfg, pio_get_dreq(pio, sm, false)); // Pace transfers based on PIO samples availabilty
    channel_config_set_read_increment(&afe_dma_channel_cfg, false);
    channel_config_set_write_increment(&afe_dma_channel_cfg, true);
    
    dma_channel_configure(wm8xxx_afe_capture_global.capture_dma,
        &afe_dma_channel_cfg,
        NULL,           // Destination: will be set later
        &pio->rxf[sm],  // Source
        0,              // Size: will be set later
        false
    );
}

//AFE Main calls
int wm8xxx_afe_setup(const wm8xxx_afe_config_t* config, uint sampling_rate)
{
    int res = wm8xxx_afe_spi_setup(config);

    if (res > 0) { return res; }

    wm8xxx_afe_capture_setup(config->pio, config->sm_afe_cp, sampling_rate, config->bppx, config->pin_base_afe_op, config->pin_base_afe_ctrl);
    afe_dma_prepare(config->pio, config->sm_afe_cp);

    return res;
}

void wm8xxx_afe_capture_wait() {
    dma_channel_wait_for_finish_blocking(wm8xxx_afe_capture_global.capture_dma);
}

void wm8xxx_afe_capture_stop() {
    pio_sm_set_enabled(wm8xxx_afe_capture_global.pio, wm8xxx_afe_capture_global.sm, false);
}

uint16_t wm8xxx_afe_get_gain_max() {
    wm8xxx_family *family = &wm8xxx_afe_capture_global.config.setups.afe_family;
    if (*family == family_wm819x) {
        return WM819X_GAIN_MAX;
    } else if (*family == family_wm821x) {
        return WM821X_GAIN_MAX;
    } else { return 0; }
}

void wm8xxx_afe_update_gain(uint16_t red, uint16_t green, uint16_t blue, bool commit) {
    uint16_t gain_max = wm8xxx_afe_get_gain_max();
    if (red   > gain_max) { red   = gain_max; }
    if (green > gain_max) { green = gain_max; }
    if (blue  > gain_max) { blue  = gain_max; }
    wm8xxx_family *family = &wm8xxx_afe_capture_global.config.setups.afe_family;
    if (*family == family_wm819x) {
        wm819x_afe_pga_gain_rgb_t *gain = &wm8xxx_afe_capture_global.config.setups.wm819x.pga_gain;
        gain->red   = red   &0xFF;
        gain->green = green &0xFF;
        gain->blue  = blue  &0xFF;
    } else if (*family == family_wm821x) {
        wm821x_afe_pga_gain_rgb_t *gain = &wm8xxx_afe_capture_global.config.setups.wm821x.pga_gain;
        wm821x_afe_pga_gain_rgb_t new_gain = {
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
        *gain = new_gain;
    } else {
        return;
    }

    if (commit) {
        wm8xxx_afe_spi_setup(NULL);
    }
}

uint16_t wm8xxx_afe_get_gain(color_part part) {
    uint16_t red = 0;
    uint16_t green = 0;
    uint16_t blue = 0;

    wm8xxx_family *family = &wm8xxx_afe_capture_global.config.setups.afe_family;
    if (*family == family_wm819x) {
        wm819x_afe_pga_gain_rgb_t *gain = &wm8xxx_afe_capture_global.config.setups.wm819x.pga_gain;
        red   = gain->red;
        green = gain->green;
        blue =  gain->blue;
    } else if (*family == family_wm821x) {
        wm821x_afe_pga_gain_rgb_t *gain = &wm8xxx_afe_capture_global.config.setups.wm821x.pga_gain;
        red   = ((uint16_t) (gain->red.msb   << 1)) | (gain->red.lsb   & 0x1);
        green = ((uint16_t) (gain->green.msb << 1)) | (gain->green.lsb & 0x1);
        blue =  ((uint16_t) (gain->blue.lsb  << 1)) | (gain->blue.lsb  & 0x1);
    }
   
    switch (part) {
        case color_part_red:   return red;
        case color_part_green: return green;
        case color_part_blue:  return blue;
        default: return (red + green + blue) / 3;
    }
}

wm8xxx_afe_offset_dac_t *wm8xxx_afe_get_offset_register() {
    wm8xxx_family *family = &wm8xxx_afe_capture_global.config.setups.afe_family;
    if (*family == family_wm819x) {
        return &(wm8xxx_afe_capture_global.config.setups.wm819x.offset_dac);
    } else if (*family == family_wm821x) {
        return &(wm8xxx_afe_capture_global.config.setups.wm821x.offset_dac);
    } else {
        return NULL;
    }
}

void wm8xxx_afe_update_offset(uint8_t red, uint8_t green, uint8_t blue, bool commit) {
    wm8xxx_afe_offset_dac_t new_offset = {
        .red = red,
        .green = green,
        .blue = blue
    };

    wm8xxx_afe_offset_dac_t *offset_dac = wm8xxx_afe_get_offset_register();
    if (offset_dac == NULL) {
        return;
    }
    *offset_dac = new_offset;
    
    if (commit) {
        wm8xxx_afe_spi_setup(NULL);
    }
}

uint8_t wm8xxx_afe_get_offset(color_part part) {
    wm8xxx_afe_offset_dac_t *offset_dac = wm8xxx_afe_get_offset_register();

    if (offset_dac == NULL) {
        return 0;
    }
    switch (part) {
        case color_part_red:   return offset_dac->red;
        case color_part_green: return offset_dac->green;
        case color_part_blue:  return offset_dac->blue;
        default: return ((uint16_t)offset_dac->red + (uint16_t)offset_dac->green + (uint16_t)offset_dac->blue) / 3;
    }
}

wm8xxx_afe_setup_3_t *wm8xxx_afe_get_setup3_register() {
    wm8xxx_family *family = &wm8xxx_afe_capture_global.config.setups.afe_family;
    if (*family == family_wm819x) {
        return &(wm8xxx_afe_capture_global.config.setups.wm819x.setup3);
    } else if (*family == family_wm821x) {
        return &(wm8xxx_afe_capture_global.config.setups.wm821x.setup3);
    } else {
        return NULL;
    }
}

void wm8xxx_afe_update_negative_offset(uint8_t value, bool commit) {
    if (value > WM8XXX_NEG_OFFSET_MAX) { value = WM8XXX_NEG_OFFSET_MAX; }
    wm8xxx_afe_setup_3_t *setup3 = wm8xxx_afe_get_setup3_register();
    if (setup3 == NULL) {
        return;
    }
    setup3->rlc_dac = value;

    if (commit) {
        wm8xxx_afe_spi_setup(NULL);
    }
}

uint8_t wm8xxx_afe_get_negative_offset() {
    wm8xxx_afe_setup_3_t *setup3 = wm8xxx_afe_get_setup3_register();
    if (setup3 == NULL) {
        return 0;
    }
    return setup3->rlc_dac & WM8XXX_NEG_OFFSET_BITS;
}

wm8xxx_family wm8xxx_afe_get_current_family() {
    if (wm8xxx_afe_capture_global.config.setups.afe_family >= family_max) {
        wm8xxx_afe_capture_global.config.setups.afe_family = family_unknown;
    }
    return wm8xxx_afe_capture_global.config.setups.afe_family;
}

const char *wm8xxx_afe_get_current_family_str() {
    return afe_family_str[wm8xxx_afe_capture_global.config.setups.afe_family];
}
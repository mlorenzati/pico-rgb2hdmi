#include "wm8213Afe.h"
#include "hardware/gpio.h"
#include "wm8213Afe.pio.h"
//#include "hardware/structs/bus_ctrl.h"

//Local registers
static uint        afe_cs = -1;
static spi_inst_t *afe_spi;
static bool        spi_write_only_mode = true;
uint16_t           dummy_dma_read;

//Global Afe Capture vars
wm8213_afe_capture_t wm8213_afe_capture_global;

// AFE SPI Related
void wm8213_enable_cs(bool val) {
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
void wm8213_afe_capture_setup(PIO pio, uint sm, uint sampling_rate, uint op_pins, uint control_pins) {
    //Setup OP and sample ports on PIO
    wm8213_afe_capture_global.pio = pio;
    wm8213_afe_capture_global.sm =  sm;
    wm8213_afe_capture_global.sampling_rate = sampling_rate;
    wm8213_afe_capture_global.op_pins = op_pins;
    wm8213_afe_capture_global.control_pins = control_pins;
    uint offset =  pio_add_program(wm8213_afe_capture_global.pio, 
        sampling_rate > AFE_SAMPLING_LIMIT ?  &afe_capture_565_inverted_program : &afe_capture_565_program);
    afe_capture_565_program_init(wm8213_afe_capture_global.pio, wm8213_afe_capture_global.sm, offset, wm8213_afe_capture_global.sampling_rate, op_pins, control_pins);
    
    //Give DMA R/W priority over the Bus
    //bus_ctrl_hw->priority = BUSCTRL_BUS_PRIORITY_DMA_W_BITS | BUSCTRL_BUS_PRIORITY_DMA_R_BITS;
}

void wm8213_afe_capture_update_sampling_rate(uint sampling_rate) {
    wm8213_afe_capture_setup(wm8213_afe_capture_global.pio, wm8213_afe_capture_global.sm, sampling_rate, wm8213_afe_capture_global.op_pins, wm8213_afe_capture_global.control_pins);
}

// AFE DMA related
void afe_dma_prepare(PIO pio, uint sm) {
    wm8213_afe_capture_global.capture_dma = dma_claim_unused_channel(true);
    wm8213_afe_capture_global.front_porch_dma = dma_claim_unused_channel(true);

    dma_channel_config front_porch_dma_channel_cfg = dma_channel_get_default_config(wm8213_afe_capture_global.front_porch_dma);
    channel_config_set_transfer_data_size(&front_porch_dma_channel_cfg, DMA_SIZE_16);   //Transfer 16bits words that are shifted by pio
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
    channel_config_set_transfer_data_size(&afe_dma_channel_cfg, DMA_SIZE_16);   //Transfer 16bits words that are shifted by pio
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
int wm8213_afe_setup(const wm8213_afe_config_t* config)
{
    int res = wm8213_afe_spi_setup(config);

    if (res > 0) { return res; }

    wm8213_afe_capture_setup(config->pio, config->sm_afe_cp, config->sampling_rate_afe, config->pin_base_afe_op, config->pin_base_afe_ctrl);
    afe_dma_prepare(config->pio, config->sm_afe_cp);

    return res;
}

void wm8213_afe_capture_wait() {
    dma_channel_wait_for_finish_blocking(wm8213_afe_capture_global.capture_dma);
}

void wm8213_afe_capture_stop() {
    pio_sm_set_enabled(wm8213_afe_capture_global.pio, wm8213_afe_capture_global.sm, false);
}
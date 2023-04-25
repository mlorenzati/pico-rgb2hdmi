//System defined includes
#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/sem.h"
#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "hardware/sync.h"
#include "hardware/vreg.h"

//Library related includes
#include "dvi.h"
#include "dvi_serialiser.h"
#include "keyboard.h"
#include "graphics.h"
#include "gui.h"

//System configuration includes
#include "version.h"
#include "common_configs.h"
#include "math.h"

// System config definitions
// TMDS bit clock 252 MHz
// DVDD 1.2V (1.1V seems ok too)
#define FRAME_HEIGHT        240
#define FRAME_WIDTH_8_BITS  640
#define FRAME_WIDTH_16_BITS 320
uint8_t genbuf[FRAME_HEIGHT][FRAME_WIDTH_8_BITS];
uint8_t  *framebuf_8  = GET_RGB8_BUFFER(genbuf);
uint16_t *framebuf_16 = GET_RGB16_BUFFER(genbuf);
#define REFRESH_RATE 50
#define VREG_VSEL VREG_VOLTAGE_1_20
#define DVI_TIMING dvi_timing_640x480p_60hz

// --------- Global register start --------- 
struct dvi_inst dvi0;
bool symbols_per_word = 0; //0: 1 symbol (320x240@16), 1: 2 symbols(640x240@8)
uint gpio_pins[3] = { KEYBOARD_PIN_UP, KEYBOARD_PIN_DOWN, KEYBOARD_PIN_ACTION };
const uint LED_PIN = PICO_DEFAULT_LED_PIN;
bool blink = true;
static uint hdmi_scanline = 2;
static graphic_ctx_t graphic_ctx = {
	.height = FRAME_HEIGHT,
	.video_buffer = genbuf,
	.parent = NULL
};
const uint colors_8[]  = {color_8_dark_gray,  color_8_light_gray,  color_8_white,  color_8_black,  color_8_mid_gray,  color_8_green };
const uint colors_16[] = {color_16_dark_gray, color_16_light_gray, color_16_white, color_16_black, color_16_mid_gray, color_16_green };

gui_object_t *focused_object = NULL;

// --------- Global register end --------- 

void __not_in_flash_func(core1_main)() {
	dvi_register_irqs_this_core(&dvi0, DMA_IRQ_0);
	dvi_start(&dvi0);
	if (symbols_per_word) {
        dvi_scanbuf_main_16bpp(&dvi0); 
    } else {
        dvi_scanbuf_main_8bpp(&dvi0);
    }
	__builtin_unreachable();
}

static inline void core1_scanline_callback() {
	void *bufptr = NULL;
	while (queue_try_remove_u32(&dvi0.q_colour_free, &bufptr));
    if (dvi0.ser_cfg.symbols_per_word) {
        bufptr = &framebuf_16[graphic_ctx.width * hdmi_scanline];
    } else {
        bufptr = &framebuf_8[graphic_ctx.width * hdmi_scanline];
    }
	
	queue_add_blocking_u32(&dvi0.q_colour_valid, &bufptr);
	if (++hdmi_scanline >= FRAME_HEIGHT) {
    	hdmi_scanline = 0;
	}
}

void on_keyboard_event(keyboard_status_t keys) {
	if (focused_object == NULL) {
		return;
	}
	if (keys.key1_down) {
		focused_object = gui_activate(focused_object);
	} else if (keys.key1_up) {
		focused_object = gui_deactivate(focused_object);
	} else if (keys.key2_down) {
		if (focused_object->base.status.navigable) {
			focused_object = gui_next_focus(focused_object);
		} else {
			gui_set_add(focused_object);
		}
	} else if (keys.key3_down) {
		if (focused_object->base.status.navigable) {
			focused_object = gui_previous_focus(focused_object);
		} else {
			gui_set_sub(focused_object);
		}
	} else if (keys.key2_up && !focused_object->base.status.navigable) {
		gui_clear_add(focused_object);
	} else if (keys.key3_up && !focused_object->base.status.navigable) {
		gui_clear_sub(focused_object);
	}
}

int main() {
	vreg_set_voltage(VREG_VSEL);
	sleep_ms(10);

	// Run system at TMDS bit clock
	set_sys_clock_khz(DVI_TIMING.bit_clk_khz, true);

    stdio_init_all();
	gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

	printf("Initializing keyboard\n");
	keyboard_initialize(gpio_pins, 3, KEYBOARD_REFRESH_RATE_MS, KEYBOARD_REPEAT_RATE_MS, on_keyboard_event);

	printf("Configuring DVI\n");

	dvi0.timing = &DVI_TIMING;
	dvi0.ser_cfg = DVI_DEFAULT_SERIAL_CONFIG;
    dvi0.ser_cfg.symbols_per_word = symbols_per_word;
    dvi0.scanline_callback = core1_scanline_callback;
	dvi_init(&dvi0, next_striped_spin_lock_num(), next_striped_spin_lock_num());

	// Once we've given core 1 the framebuffer, it will just keep on displaying
	// it without any intervention from core 0

	//Prepare for the first time the two initial lines
	void  *bufptr = NULL;
	queue_add_blocking_u32(&dvi0.q_colour_valid, &bufptr);
	bufptr += graphic_ctx.width;
	queue_add_blocking_u32(&dvi0.q_colour_valid, &bufptr);

	printf("Core 1 start\n");
	multicore_launch_core1(core1_main);
	
	printf("%s version - GUI Test %s started!\n", PROJECT_NAME, PROJECT_VER);

	printf("Start rendering\n");

	//GUI Props
	gui_list_t colors_8_list = initalizeGuiList(colors_8);
    gui_list_t colors_16_list = initalizeGuiList(colors_16);
    gui_list_t colors_list = symbols_per_word ? colors_16_list : colors_8_list;
    
	gui_properties_t common_nshared_props = {
        .focusable  = 1,
        .alignment  = gui_align_center,
        .horiz_vert = gui_orientation_vertical,
        .padding    = 1,
        .shared     = 0,
        .border     = 1 
	};
	gui_properties_t spinbox_props = {
        .focusable  = 1,
		.alignment = gui_align_center,
		.horiz_vert = gui_orientation_horizontal,
		.padding = 1,
		.shared = 0,
		.border = 1
	};

	// GUI Objects
	uint slider_value = 0;
	uint spinbox_value = 0;
    graphic_ctx.bppx  = symbols_per_word ? rgb_16_565 : rgb_8_332;
    graphic_ctx.width = symbols_per_word ? FRAME_WIDTH_16_BITS : FRAME_WIDTH_8_BITS;
	
	gui_object_t window = gui_create_window(&graphic_ctx, 0, 0, graphic_ctx.width, graphic_ctx.height, &colors_list, common_nshared_props);

	gui_object_t group_elements[] = { 
		gui_create_button(&graphic_ctx,  0, 0, 100, 12, &colors_list, common_nshared_props, "Button 1"),
		gui_create_button(&graphic_ctx,  0, 0, 100, 12, &colors_list, common_nshared_props, "Button 2"),
		gui_create_slider(&graphic_ctx,  0, 0, 100, 16, &colors_list, common_nshared_props, &slider_value),
		gui_create_button(&graphic_ctx,  0, 0, 100, 12, &colors_list, common_nshared_props, "Button 3"),
		gui_create_button(&graphic_ctx,  0, 0, 100, 12, &colors_list, common_nshared_props, "Button 4"),
		gui_create_spinbox(&graphic_ctx, 0, 0, 100, 12, &colors_list, spinbox_props, &spinbox_value)
	};

	gui_list_t group_list = initalizeGuiList(group_elements);
	gui_object_t group = gui_create_group(&graphic_ctx, 0, 0,
		gui_sum(&group_list, common_nshared_props, gui_coord_width), 
		gui_sum(&group_list, common_nshared_props, gui_coord_height), 
		&colors_list, common_nshared_props , &group_list);

	void label_print(print_delegate_t printer) {
		printer("This is an event based text:\nSlider = %d\nSpinbox = %d", slider_value, spinbox_value);
	};

	gui_object_t label = gui_create_label(&graphic_ctx, 
		group.base.x + group.base.width + 1,
		group.base.y,
		group.base.width * 3, group.base.height,  &colors_list, common_nshared_props, label_print);

	gui_status_t buttons_status = { .activated = 1, .add = 1, .substract = 1 };
	bool on_slider_event(gui_status_t status, gui_base_t *origin, gui_object_t *destination) {
		if (status.activated && !origin->status.activated) {
			destination->base.status.navigable = !destination->base.status.navigable;
		} else if (status.add && slider_value < GUI_BAR_100PERCENT) {
			slider_value += 100;
		} else if (status.substract && slider_value != 0) {
			slider_value -= 100;
		}
		destination->base.status.data_changed = 1;
		
		return true;
	}

	bool on_spinbox_event(gui_status_t status, gui_base_t *origin, gui_object_t *destination) {
		if (status.activated && !origin->status.activated) {
			destination->base.status.navigable = !destination->base.status.navigable;
		} else if (status.add && spinbox_value < 100) {
			spinbox_value += 1;
		} else if (status.substract && spinbox_value != 0) {
			spinbox_value -= 1;
		}
		destination->base.status.data_changed = 1;
		
		return true;
	}

	//Event Subscriptions
	gui_status_t data_status = { .data_changed = 1 };
	gui_event_subscribe(buttons_status, &group_elements[2].base, &group_elements[2], on_slider_event);
	gui_event_subscribe(buttons_status, &group_elements[5].base, &group_elements[5], on_spinbox_event);
	gui_event_subscribe(data_status, &group_elements[2].base, &label, NULL); //Just trigger a redraw, no callback needed
	gui_event_subscribe(data_status, &group_elements[5].base, &label, NULL); //Just trigger a redraw, no callback needed

	gui_obj_draw(window);
	gui_obj_draw(group);
	gui_obj_draw(label);
	focused_object = gui_focused(&group_elements[0]);

	while (1)
	{
		sleep_ms(50);
	}
	__builtin_unreachable();
}


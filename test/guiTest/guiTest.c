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
#define FRAME_HEIGHT 240
#if DVI_SYMBOLS_PER_WORD == 2
	//With 2 repeated symbols per word, we go for 320 pixels width and 16 bits per pixel
	#define FRAME_WIDTH 320
	uint16_t framebuf[FRAME_HEIGHT][FRAME_WIDTH];
    #define GUI_OVERLAY_BBPX    rgb_16_565
#else
	//With no repeated symbols per word, we go for 640 pixels width and 8 bits per pixel
	#define FRAME_WIDTH 640
	uint8_t framebuf[FRAME_HEIGHT][FRAME_WIDTH];
    #define GUI_OVERLAY_BBPX    rgb_8_332
#endif

#define REFRESH_RATE 50
#define VREG_VSEL VREG_VOLTAGE_1_20
#define DVI_TIMING dvi_timing_640x480p_60hz

// --------- Global register start --------- 
struct dvi_inst dvi0;
uint gpio_pins[3] = { KEYBOARD_PIN_UP, KEYBOARD_PIN_DOWN, KEYBOARD_PIN_ACTION };
const uint LED_PIN = PICO_DEFAULT_LED_PIN;
bool blink = true;
static uint hdmi_scanline = 2;
static graphic_ctx_t graphic_ctx = {
	.width = FRAME_WIDTH,
	.height = FRAME_HEIGHT,
	.video_buffer = framebuf,
	.bppx = GUI_OVERLAY_BBPX,
	.parent = NULL
};

gui_object_t *focused_object = NULL;

// --------- Global register end --------- 

void __not_in_flash_func(core1_main)() {
	dvi_register_irqs_this_core(&dvi0, DMA_IRQ_0);
	dvi_start(&dvi0);
	#if DVI_SYMBOLS_PER_WORD == 2
		dvi_scanbuf_main_16bpp(&dvi0);
	#else
		dvi_scanbuf_main_8bpp(&dvi0);
	#endif
	__builtin_unreachable();
}

static inline void core1_scanline_callback() {
	#if DVI_SYMBOLS_PER_WORD == 2
		uint16_t *bufptr = NULL;
	#else
		uint8_t *bufptr  = NULL;
	#endif
	while (queue_try_remove_u32(&dvi0.q_colour_free, &bufptr));
	bufptr = &framebuf[hdmi_scanline][0];
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
	dvi0.scanline_callback = core1_scanline_callback;
	dvi_init(&dvi0, next_striped_spin_lock_num(), next_striped_spin_lock_num());

	// Once we've given core 1 the framebuffer, it will just keep on displaying
	// it without any intervention from core 0

	//Prepare for the first time the two initial lines
	#if DVI_SYMBOLS_PER_WORD == 2
		uint16_t *bufptr = NULL;
	#else
		uint8_t  *bufptr = NULL;
	#endif
	queue_add_blocking_u32(&dvi0.q_colour_valid, &bufptr);
	bufptr += FRAME_WIDTH;
	queue_add_blocking_u32(&dvi0.q_colour_valid, &bufptr);

	printf("Core 1 start\n");
	multicore_launch_core1(core1_main);
	
	printf("%s version - GUI Test %s started!\n", PROJECT_NAME, PROJECT_VER);

	printf("Start rendering\n");

	//GUI Props
	uint colors[] = {color_dark_gray, color_light_gray, color_white, color_black, color_mid_gray, color_green };
	gui_list_t colors_list = initalizeGuiList(colors);
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
	
	gui_object_t window = gui_create_window(&graphic_ctx, 0, 0, FRAME_WIDTH, FRAME_HEIGHT, &colors_list, common_nshared_props);
	
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


#include <htc.h>
#include "st7565.h"
#include "delay.h"

void glcd_pixel(unsigned char x, unsigned char y, unsigned char colour) {

#ifdef ST7565_USE_DIRTY
	// Todo: Set the page dirty, so we only refresh dirty pages
#endif

	if (x > SCREEN_WIDTH || y > SCREEN_HEIGHT) return;

	// Real screen coordinates are 0-63, not 1-64.
	x -= 1;
	y -= 1;

	if (colour) {
		glcd_buffer[x + ((y / 8) * 128)] |= 1 << (y % 8);;
	} else {
		glcd_buffer[x + ((y / 8) * 128)] &= 0xFF ^ 1 << (y % 8);
	}
}

void glcd_blank() {
	for (int y = 0; y < 8; y++) {
		glcd_command(GLCD_CMD_SET_PAGE | y);

		// Reset column to 0 (the left side)
		glcd_command(GLCD_CMD_COLUMN_LOWER);
		glcd_command(GLCD_CMD_COLUMN_UPPER);

		// We iterate to 132 as the internal buffer is 65*132, not
		// 64*124.
		for (int x = 0; x < 132; x++) {
			glcd_data(0x00);
		}
	}
}

void glcd_refresh() {
	for (int y = 0; y < 8; y++) {
		glcd_command(GLCD_CMD_SET_PAGE | y);

		// Reset column to the left side.  The internal memory of the 
		// screen is 132*64, we need to account for this if the display 
		// is flipped.
		if (glcd_flipped) {
			glcd_command(GLCD_CMD_COLUMN_LOWER | 4);
		} else {
			glcd_command(GLCD_CMD_COLUMN_LOWER);
		}
		glcd_command(GLCD_CMD_COLUMN_UPPER);

		for (int x = 0; x < 128; x++) {
			glcd_data(glcd_buffer[y * 128 + x]);
		}
	}
}

void glcd_init() {

	// Select the chip
	GLCD_CS1 = 0;

	GLCD_RESET = 0;

	// Datasheet says "wait for power to stabilise" but gives
	// no specific time!
	DelayMs(50);

	GLCD_RESET = 1;

	// Datasheet says max 1ms here
	//DelayMs(1);

	// Set LCD bias to 1/9th
	glcd_command(GLCD_CMD_BIAS_9);

	// Horizontal output direction (ADC segment driver selection)
	glcd_command(GLCD_CMD_HORIZONTAL_NORMAL);

	// Vertical output direction (common output mode selection)
	glcd_command(GLCD_CMD_VERTICAL_REVERSE);

	// The screen is the "normal" way up
	glcd_flipped = 0;

	// Set internal resistor.  A suitable middle value is used as
	// the default.
	glcd_command(GLCD_CMD_RESISTOR | 0x3);

	// Power control setting (datasheet step 7)
	// Note: Skipping straight to 0x7 works with my hardware.
//	glcd_command(GLCD_CMD_POWER_CONTROL | 0x4);
//	DelayMs(50);
//	glcd_command(GLCD_CMD_POWER_CONTROL | 0x6);
//	DelayMs(50);
	glcd_command(GLCD_CMD_POWER_CONTROL | 0x7);
//	DelayMs(10);

	// Volume set (brightness control).  A middle value is used here
	// also.
	glcd_command(GLCD_CMD_VOLUME_MODE);
	glcd_command(31);

	// Reset start position to the top
	glcd_command(GLCD_CMD_DISPLAY_START);

	// Unselect the chip
	GLCD_CS1 = 1;
}

void glcd_data(unsigned char data) {

	// A0 is high for display data
	GLCD_A0 = 1;

	// Select the chip
	GLCD_CS1 = 0;

	for (int n = 0; n < 8; n++) {

		if (data & 0x80) {
			GLCD_SDA = 1;
		} else {
			GLCD_SDA = 0;
		}

		// Pulse SCL
		GLCD_SCL = 1;
		// TODO: Check if timings allow us to omit this NOP
		asm("nop");
		GLCD_SCL = 0;

		data <<= 1;
	}

	// Unselect the chip
	GLCD_CS1 = 1;

}

void glcd_command(char command) {

	// A0 is low for command data
	GLCD_A0 = 0;

	// Select the chip
	GLCD_CS1 = 0;

	for (int n = 0; n < 8; n++) {

		if (command & 0x80) {
			GLCD_SDA = 1;
		} else {
			GLCD_SDA = 0;
		}

		// Pulse SCL
		GLCD_SCL = 1;
		// TODO: Check if timings allow us to omit this NOP
		asm("nop");
		GLCD_SCL = 0;

		command <<= 1;
	}

	// Unselect the chip
	GLCD_CS1 = 1;
}

void glcd_flip_screen(unsigned char flip) {
	if (flip) {
		glcd_command(GLCD_CMD_HORIZONTAL_NORMAL);
		glcd_command(GLCD_CMD_VERTICAL_REVERSE);
		glcd_flipped = 0;
	} else {
		glcd_command(GLCD_CMD_HORIZONTAL_REVERSE);
		glcd_command(GLCD_CMD_VERTICAL_NORMAL);
		glcd_flipped = 1;
	}
}

void glcd_inverse_screen(unsigned char inverse) {
	if (inverse) {
		glcd_command(GLCD_CMD_DISPLAY_REVERSE);
	} else {
		glcd_command(GLCD_CMD_DISPLAY_NORMAL);
	}
}

void glcd_test_card() {
	unsigned char p = 0xF0;	

	for (int n=1; n<=1024; n++) {
		glcd_buffer[n - 1] = p;

		if (n % 4 == 0) {
			unsigned char q = p;
			p = p << 4;
			p |= q >> 4;
		}
	}

	glcd_refresh();
}

void glcd_contrast(char resistor_ratio, char contrast) {
	if (resistor_ratio > 7 || contrast > 63) return;

	glcd_command(GLCD_CMD_RESISTOR | resistor_ratio);
	glcd_command(GLCD_CMD_VOLUME_MODE);
	glcd_command(contrast);
}

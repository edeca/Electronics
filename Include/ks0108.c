#include <htc.h>
#include "ks0108.h"

// Globals for the current row and column address
unsigned char glcd_row;
unsigned char glcd_col;

void _glcd_wait(void)
{
	// Set the data port to input mode
	GLCD_DATA_TRIS = 0xff;
	GLCD_RS = 0;
	GLCD_RW = 1;
	
	// Wait for the BUSY flag to clear
	while (_glcd_read() & GLCD_BUSY_FLAG);
	
	// If both chips are currently selected, wait for each
	// one in turn
	if (GLCD_CS1 == 1 && GLCD_CS2 == 1) {
		// Wait for first controller		
		GLCD_CS2 = 0;
		while (_glcd_read() & 0x80);
		GLCD_CS2 = 1;
		
		// Wait for second controller
		GLCD_CS1 = 0;
		while (_glcd_read() & 0x80);
		GLCD_CS1 = 1;
	} else {
		while (_glcd_read() & 0x80);
	}
	
	// Restore data port to output
	GLCD_DATA_TRIS = 0x00;
	return;
}

void glcd_command(unsigned char cmd) 
{
	_glcd_wait();
	
	GLCD_DATA_PORT = cmd;
	GLCD_RS = 0;
	GLCD_RW = 0;
	GLCD_EN = 1;
	asm("nop");
	GLCD_EN = 0;
	
	return;
}

void glcd_write_data(unsigned char data)
{
	_glcd_wait();
	
	GLCD_DATA_PORT = data;
	GLCD_RS = 1;
	GLCD_RW = 0;
	GLCD_EN = 1;
	asm("nop");
	GLCD_EN = 0;	
	return;
}

unsigned char _glcd_read(void) {
	unsigned char data;
	GLCD_EN = 1;
	data = GLCD_DATA_PORT;
	GLCD_EN = 0;
	return data;
}

void _glcd_set_position()
{
	if (glcd_col > 63) {
		GLCD_CS1 = 0;
		GLCD_CS2 = 1;
	} else {
		GLCD_CS1 = 1;
		GLCD_CS2 = 0;
	}
	
	glcd_command(0x40 | glcd_col);
	glcd_command(0xB8 | (glcd_row >> 3));
}

void glcd_clear_screen(void)
{
	unsigned char i, j;
	
	GLCD_CS1 = 1;
	GLCD_CS2 = 1;
		
	for(i=0; i<8; i++) {
		glcd_command(0x40);		//y=0	
		glcd_command(0xb8+i);	//x=0	
		for(j=0; j<64; j++) {
			glcd_write_data(0x00);
		}
	}
}

void glcd_fill_screen(void)
{
	unsigned char i, j;
	
	GLCD_CS1 = 1;
	GLCD_CS2 = 1;
	
	for(i=0; i<8; i++) {
		glcd_command(0x40);		//y=0	
		glcd_command(0xb8+i);	//x=0	
		for(j=0; j<64; j++) {
			glcd_write_data(0xff);
		}
	}
}

void glcd_goto(unsigned char x, unsigned char y)
{
	glcd_row = x;
	glcd_col = y;
	_glcd_set_position();	
	return;
}

/* Move one column right across the screen, taking into account
 * moving between controllers or over the last column
 */
void glcd_move_right(void) {
	// Moving right takes us across the middle, switch to second
	// controller
	if (++glcd_col == 64) {
		_glcd_set_position();
	}
	// Too far right, move one row down and back to the
	// start
	if (glcd_col == 128) {
		glcd_col = 0;
		glcd_row += 8;
		glcd_row &= 0x3F;
		_glcd_set_position();
	}
}

void glcd_char(unsigned char character, unsigned char inverted) {
	unsigned char i, d;
	
	// TODO: Handle CR and LF
	
	for (i = 0; i < 7; i++) {
		d = Font[character - 32][i];
		
		// 0x55 is used as padding and allows variable
		// width characters
		if (d != 0x55) {
			if (inverted)
				glcd_write_data(d);
			else
				glcd_write_data(d ^ 0xFF);
			glcd_move_right();
		}
	}
	
	// Write an empty column after the letter 
	if (inverted)
		glcd_write_data(0xFF);
	else
		glcd_write_data(0x00);
	glcd_move_right();
	return;
}

void glcd_string(const char *string, unsigned char inverted)
{
	// TODO: Implement 0x16 as "move position" marker?
	
	while (*string != 0x00) {
		glcd_char(*string++, inverted);
	}
}

void glcd_init(void)
{
	GLCD_RW_TRIS = 0; 
	GLCD_EN_TRIS = 0;
	GLCD_RS_TRIS = 0;
	GLCD_CS1_TRIS = 0;
	GLCD_CS2_TRIS = 0;
	GLCD_DATA_TRIS = 0;
	
	GLCD_CS1 = 1;
	GLCD_CS2 = 1;
	
	glcd_command(GLCD_DISPLAY_ON);		// Turn display on
	glcd_command(0xc0);		// Set display start line to 0
	glcd_clear_screen();
	glcd_goto(0,0);
}

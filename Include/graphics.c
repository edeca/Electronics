// stdlib.h for abs()
#include <stdlib.h>

#include "graphics.h"
#include "main.h"
#include "st7565.h"

#include "usart.h"	// DEBUG
#include <stdio.h>	// DEBUG

// TODO: 

bounding_box_t glcd_string_new(char *string, unsigned char x, unsigned char y, unsigned char *font) {
	bounding_box_t ret; // = { x, y, 0, y };
	bounding_box_t tmp;

	ret.x1 = x;
	ret.y1 = y;

	// BUG: As we move right between chars we don't actually wipe the space
	while (*string != 0) {
		tmp = glcd_char_new(*string++, x, y, font);

		// Leave a single space between characters
		x = tmp.x2 + 2;
	}

	ret.x2 = tmp.x2;
	ret.y2 = tmp.y2;

	return ret;
}

bounding_box_t glcd_char_new(unsigned char c, unsigned char x, unsigned char y, unsigned char *font) {
	unsigned short pos;
	unsigned char width, height;
	bounding_box_t ret;
//	unsigned char k = 0;

//printf("Called for character %c\r\n", c);

	ret.x1 = x;
	ret.y1 = y;
	ret.x2 = x;
	ret.y2 = y;

	// Read first byte, should be 0x01 for proportional
	if (font[0] != 0x01) return ret;

	// Check second byte, should be 0x02 for "vertical ceiling"
	if (font[1] != 0x02) return ret;

	// Check that font start + number of bitmaps contains c
	if (!(c >= font[2] && c <= font[2] + font[3])) return ret;

	// Adjust for start position of font vs. the char passed
	c -= font[2];

	// Work out where in the array the character is
	pos = font[c * 2 + 5];
	pos <<= 8;
	pos |= font[c * 2 + 6];

	// Height is stored in the header
	height = font[4];

	// Read first byte from this position, this gives letter width
	width = font[pos];

//printf("letter %c has height %u and width %u, position in array is %u\r\n", c, height, width, pos);

	// Draw left to right
	unsigned char i;
	for (i = 0; i < width; i++) {

		// Draw top to bottom
		for (unsigned char j = 0; j < height; j++) {
			// Increment one data byte every 8 bits, or
			// at the start of a new column  HiTech optimizes
			// the modulo, so no need to try and avoid it.
			if (j % 8 == 0) pos++;

			if (font[pos] & 1 << (j % 8)) {
				glcd_pixel(x + i, y + j, 1);
			} else {
				glcd_pixel(x + i, y + j, 0);
			}
		}
	}

	//return x + width;
	ret.x2 = ret.x1 + width - 1;
	ret.y2 = ret.y1 + height;

	return ret;
}

unsigned char text_height(unsigned char *string, unsigned char *font) {
	// TODO: Possibly work out the actual pixel height.  For letters with
	//       descenders (like 'g') are taller than letters without (like 'k')

	// Height is stored in the header
	return font[4];
}

unsigned char text_width(unsigned char *string, unsigned char *font) {
	unsigned char width = 0;
	unsigned short pos;
	unsigned char c;

	// TODO: Implement for fixed width fonts

	// Read first byte, should be 0x01 for proportional
	if (font[0] != 0x01) return 0;

	while (*string != 0) {
		c = *string++;
	
		// Check that font start + number of bitmaps contains c
		// TODO: Should we continue here but add 0 to width?
		if (!(c >= font[2] && c <= font[2] + font[3])) return 0;
	
		// Adjust for start position of font vs. the char passed
		c -= font[2];
	
		// Work out where in the array the character is
		pos = font[c * 2 + 5];
		pos <<= 8;
		pos |= font[c * 2 + 6];
	
		// Read first byte from this position, this gives letter width
		width += font[pos];

		// Allow for space between letters
		width++;
	}

	// The last letter shouldn't have a space after it
	return width - 1;
}


/*
unsigned char text_width(char *string) {
	// Loop through the whole string, adding up the width of each character
	// Allow for the space between characters

	unsigned char width = 0;

	while (*string != 0) {
		for (int i = 0; i < 7; i++) {
			if (Font[*string - 32][i] == 0x55) break;
			width += 1;
		}

		// Allow for space between letters
		width += 1;
		string++;
	}

	// We don't want a space after the last character
	return width - 1;
}
*/

void draw_rectangle(int x1, int y1, int x2, int y2, char colour)
{
	// Top
	draw_line(x1, y1, x2, y1, colour);
	// Left
	draw_line(x1, y1, x1, y2, colour);
	// Bottom
	draw_line(x1, y2, x2, y2, colour);
	// Right
	draw_line(x2, y1, x2, y2, colour);
}


// A rounded box
void draw_box(int x1, int y1, int x2, int y2, char colour)
{
	// Top
	draw_line(x1 + 1, y1, x2 - 1, y1, colour);
	// Left
	draw_line(x1, y1 + 1, x1, y2 - 1, colour);
	// Bottom
	draw_line(x1 + 1, y2, x2 - 1, y2, colour);
	// Right
	draw_line(x2, y1 + 1, x2, y2 - 1, colour);
}


void draw_line(int x1, int y1, int x2, int y2, char colour)
{
	int xinc1, yinc1, den, num, numadd, numpixels, curpixel, xinc2, yinc2;

	int deltax = abs(x2 - x1);    	// The difference between the x's
	int deltay = abs(y2 - y1);    	// The difference between the y's
	int x = x1;                   	// Start x off at the first pixel
	int y = y1;                   	// Start y off at the first pixel
	
	if (x2 >= x1)             	// The x-values are increasing
	{
	  xinc1 = 1;
	  xinc2 = 1;
	}
	else                      	// The x-values are decreasing
	{
	  xinc1 = -1;
	  xinc2 = -1;
	}
	
	if (y2 >= y1)             	// The y-values are increasing
	{
	  yinc1 = 1;
	  yinc2 = 1;
	}
	else                      	// The y-values are decreasing
	{
	  yinc1 = -1;
	  yinc2 = -1;
	}
	
	if (deltax >= deltay)     	// There is at least one x-value for every y-value
	{
	  xinc1 = 0;              	// Don't change the x when numerator >= denominator
	  yinc2 = 0;              	// Don't change the y for every iteration
	  den = deltax;
	  num = deltax / 2;
	  numadd = deltay;
	  numpixels = deltax;     	// There are more x-values than y-values
	}
	else                      	// There is at least one y-value for every x-value
	{
	  xinc2 = 0;              	// Don't change the x for every iteration
	  yinc1 = 0;              	// Don't change the y when numerator >= denominator
	  den = deltay;
	  num = deltay / 2;
	  numadd = deltax;
	  numpixels = deltay;     	// There are more y-values than x-values
	}
	
	for (curpixel = 0; curpixel <= numpixels; curpixel++)
	{
	  glcd_pixel(x, y, colour);         	// Draw the current pixel
	  num += numadd;          	// Increase the numerator by the top of the fraction
	  if (num >= den)         	// Check if numerator >= denominator
	  {
		num -= den;           	// Calculate the new numerator value
		x += xinc1;           	// Change the x as appropriate
		y += yinc1;           	// Change the y as appropriate
	  }
	  x += xinc2;             	// Change the x as appropriate
	  y += yinc2;             	// Change the y as appropriate
	}
}
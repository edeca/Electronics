// stdlib.h for abs()
#include <stdlib.h>

#include "graphics.h"
#include "main.h"

bounding_box_t draw_text(char *string, unsigned char x, unsigned char y, unsigned char *font, unsigned char spacing) {
	bounding_box_t ret;
	bounding_box_t tmp;

	ret.x1 = x;
	ret.y1 = y;

	spacing += 1;

	// BUG: As we move right between chars we don't actually wipe the space
	while (*string != 0) {
		tmp = draw_char(*string++, x, y, font);

		// Leave a single space between characters
		x = tmp.x2 + spacing;
	}

	ret.x2 = tmp.x2;
	ret.y2 = tmp.y2;

	return ret;
}

bounding_box_t draw_char(unsigned char c, unsigned char x, unsigned char y, unsigned char *font) {
	unsigned short pos;
	unsigned char width;
	bounding_box_t ret;

	ret.x1 = x;
	ret.y1 = y;
	ret.x2 = x;
	ret.y2 = y;

	// Read first byte, should be 0x01 for proportional
	if (font[FONT_HEADER_TYPE] != FONT_TYPE_PROPORTIONAL) return ret;

	// Check second byte, should be 0x02 for "vertical ceiling"
	if (font[FONT_HEADER_ORIENTATION] != FONT_ORIENTATION_VERTICAL_CEILING) return ret;

	// Check that font start + number of bitmaps contains c
	if (!(c >= font[FONT_HEADER_START] && c <= font[FONT_HEADER_START] + font[FONT_HEADER_LETTERS])) return ret;

	// Adjust for start position of font vs. the char passed
	c -= font[FONT_HEADER_START];

	// Work out where in the array the character is
	pos = font[c * FONT_HEADER_START + 5];
	pos <<= 8;
	pos |= font[c * FONT_HEADER_START + 6];

	// Read first byte from this position, this gives letter width
	width = font[pos];

	// Draw left to right
	unsigned char i;
	for (i = 0; i < width; i++) {

		// Draw top to bottom
		for (unsigned char j = 0; j < font[FONT_HEADER_HEIGHT]; j++) {
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

	ret.x2 = ret.x1 + width - 1;
	// TODO: Return the actual height drawn, rather than the height of the
	//		 font.
	ret.y2 = ret.y1 + height;
	ret.y2 = ret.y1 + font[FONT_HEADER_HEIGHT];

	return ret;
}

unsigned char text_height(unsigned char *string, unsigned char *font) {
	// TODO: Possibly work out the actual pixel height.  Letters with
	//       descenders (like 'g') are taller than letters without (like 'k')

	// Height is stored in the header
	return font[FONT_HEADER_HEIGHT];
}

unsigned char text_width(unsigned char *string, unsigned char *font, unsigned char spacing) {
	unsigned char width = 0;
	unsigned short pos;
	unsigned char c;

	// TODO: Implement for fixed width fonts

	// Check font type, should be 0x01 for proportional
	if (font[FONT_HEADER_TYPE] != FONT_TYPE_PROPORTIONAL) return 0;

	while (*string != 0) {
		c = *string++;
	
		// Check that font start + number of bitmaps contains c
		// TODO: Should we continue here but add 0 to width?
		if (!(c >= font[FONT_HEADER_START] && c <= font[FONT_HEADER_START] + font[FONT_HEADER_LETTERS])) return 0;
	
		// Adjust for start position of font vs. the char passed
		c -= font[FONT_HEADER_START];
	
		// Work out where in the array the character is
		pos = font[c * FONT_HEADER_START + 5];
		pos <<= 8;
		pos |= font[c * FONT_HEADER_START + 6];
	
		// Read first byte from this position, this gives letter width
		width += font[pos];

		// Allow for space between letters
		width += spacing;
	}

	// The last letter wont have a space after it
	return width - spacing;
}

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

// Implementation of Bresenham's line algorithm
//
// This code credit Tom Ootjers, originally obtained from: 
// http://tinyurl.com/czok7vx
void draw_line(int x1, int y1, int x2, int y2, char colour)
{
	int xinc1, yinc1, den, num, numadd, numpixels, curpixel, xinc2, yinc2;

	int deltax = abs(x2 - x1);    	// The difference between the x's
	int deltay = abs(y2 - y1);    	// The difference between the y's
	int x = x1;                   	// Start x off at the first pixel
	int y = y1;                   	// Start y off at the first pixel
	
	if (x2 >= x1) {             	// The x-values are increasing
	  xinc1 = 1;
	  xinc2 = 1;

    } else {          	         	// The x-values are decreasing
	  xinc1 = -1;
	  xinc2 = -1;
	}
	
	if (y2 >= y1)       	      	// The y-values are increasing
	{
	  yinc1 = 1;
	  yinc2 = 1;
	}
	else                    	  	// The y-values are decreasing
	{
	  yinc1 = -1;
	  yinc2 = -1;
	}
	
	if (deltax >= deltay)     		// There is at least one x-value for every y-value
	{
	  xinc1 = 0;              		// Don't change the x when numerator >= denominator
	  yinc2 = 0;              		// Don't change the y for every iteration
	  den = deltax;
	  num = deltax / 2;
	  numadd = deltay;
	  numpixels = deltax;     		// There are more x-values than y-values
	}
	else                      		// There is at least one y-value for every x-value
	{
	  xinc2 = 0;              		// Don't change the x for every iteration
	  yinc1 = 0;              		// Don't change the y when numerator >= denominator
	  den = deltay;
	  num = deltay / 2;
	  numadd = deltax;
	  numpixels = deltay;     		// There are more y-values than x-values
	}
	
	for (curpixel = 0; curpixel <= numpixels; curpixel++)
	{
	  glcd_pixel(x, y, colour);    	// Draw the current pixel
	  num += numadd;          		// Increase the numerator by the top of the fraction
	  if (num >= den)         		// Check if numerator >= denominator
	  {
		num -= den;           		// Calculate the new numerator value
		x += xinc1;           		// Change the x as appropriate
		y += yinc1;           		// Change the y as appropriate
	  }
	  x += xinc2;             		// Change the x as appropriate
	  y += yinc2;             		// Change the y as appropriate
	}
}


// Implementation of Bresenham's circle algorithm
void draw_circle(unsigned char centre_x, unsigned char centre_y, unsigned char radius, unsigned char colour)
{
	signed char x = 0;
	signed char y = radius;
	signed char p = 1 - radius;

	if (!radius) return;

	for (x = 0; x < y; x++) {
		if (p < 0) {
			p += x * 2 + 3;
		} else {
			p += x * 2 - y * 2 + 5;
			y--;
		}

		glcd_pixel(centre_x - x, centre_y - y, colour);
		glcd_pixel(centre_x - y, centre_y - x, colour);
		glcd_pixel(centre_x + y, centre_y - x, colour);
		glcd_pixel(centre_x + x, centre_y - y, colour);
		glcd_pixel(centre_x - x, centre_y + y, colour);
		glcd_pixel(centre_x - y, centre_y + x, colour);
		glcd_pixel(centre_x + y, centre_y + x, colour);
		glcd_pixel(centre_x + x, centre_y + y, colour);
	}
}

// Implementation of Bresenham's circle algorithm, filled.
void draw_filled_circle(unsigned char centre_x, unsigned char centre_y, unsigned char radius, unsigned char colour)
{
	signed char x = 0;
	signed char y = radius;
	signed char p = 1 - radius;

	if (!radius) return;

	for (x = 0; x < y; x++) {
		if (p < 0) {
			p += x * 2 + 3;
		} else {
			p += x * 2 - y * 2 + 5;
			y--;
		}

		draw_line(centre_x - x, centre_y - y, centre_x + x, centre_y - y, colour);
		draw_line(centre_x - y, centre_y - x, centre_x + y, centre_y - x, colour);
		draw_line(centre_x + x, centre_y + y, centre_x - x, centre_y + y, colour);
		draw_line(centre_x + y, centre_y + x, centre_x - y, centre_y + x, colour);
	}
}

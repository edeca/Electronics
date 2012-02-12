/**
 * @file   graphics.h
 * @author David <david@edeca.net>
 * @date   November, 2011
 * @brief  Header for black and white graphics library.
 * @sa     <a href="http://en.wikipedia.org/wiki/Bresenham's_line_algorithm">Bresenham's line algorithm on Wikipedia</a>
 * @details
 *
 * A graphics library for black and white graphic LCDs.  Supports lines, rectangles and text.
 *
 * Fonts are available separately in header files. 
 *
 * This requires a hardware driver for the GLCD that provides a glcd_pixel() routine.  See
 * my ST7565 library for an example.
 *
 * The benefit of this approach is that it can be ported easily to any graphic LCD.  However,
 * speed benefits could be gained by writing routines that draw whole bytes at a time.
 *
 * Example usage:
 * @code
 *    // Draw some text
 *    draw_text("Example string", 10, 10, Tahoma10);
 *
 *    // Draw some lines
 *    draw_rectangle(1, 1, 50, 50);
 * @endcode
 */
#ifndef _GRAPHICS_H_
#define _GRAPHICS_H_

#define FONT_HEADER_TYPE		0
#define FONT_HEADER_ORIENTATION	1
#define FONT_HEADER_START		2
#define FONT_HEADER_LETTERS		3
#define FONT_HEADER_HEIGHT		4

#define FONT_TYPE_FIXED			0
#define FONT_TYPE_PROPORTIONAL	1

#define FONT_ORIENTATION_VERTICAL_CEILING	2

typedef struct {
	unsigned char x1;
	unsigned char y1; 
	unsigned char x2; 
	unsigned char y2;
} bounding_box_t;

/**
 * Draw a string on the screen at a specific location.
 * 
 * @param string	The text to render
 * @param x			The x position, from 1 - SCREEN_WIDTH
 * @param y			The y position, from 1 - SCREEN_HEIGHT
 * @param font		The font used to render the text
 * @param spacing	The gap in pixels between letters
 */
bounding_box_t draw_text(char *string, unsigned char x, unsigned char y, unsigned char *font, unsigned char spacing);
/**
 * Draw a single character on the screen at a specific location.
 * 
 * @param c			The character to render
 * @param x			The x position, from 1 - SCREEN_WIDTH
 * @param y			The y position, from 1 - SCREEN_HEIGHT
 * @param font		The font used to render the text
 */
bounding_box_t draw_char(unsigned char c, unsigned char x, unsigned char y, unsigned char *font);
/**
 * Draw a simple rectangle.
 *
 * @param x1 		The x1 position, from 1 - SCREEN_WIDTH
 * @param x2 		The x2 position, from 1 - SCREEN_WIDTH
 * @param y1 		The y1 position, from 1 - SCREEN_HEIGHT
 * @param y2 		The y2 position, from 1 - SCREEN_HEIGHT
 * @param colour 	0 = OFF, any other value = ON
 */
void draw_rectangle(int x1, int y1, int x2, int y2, char colour);
/**
 * Draw a box with rounded corners.  The same as draw_rectangle(), but with corners
 * that are not filled.
 *
 * @param x1 		The x1 position, from 1 - SCREEN_WIDTH
 * @param x2 		The x2 position, from 1 - SCREEN_WIDTH
 * @param y1 		The y1 position, from 1 - SCREEN_HEIGHT
 * @param y2 		The y2 position, from 1 - SCREEN_HEIGHT
 * @param colour 	0 = OFF, any other value = ON
 */
void draw_box(int x1, int y1, int x2, int y2, char colour);
/**
 * Obtain the width of a string in pixels.
 *
 * @param string	The text to be measured
 * @param font		The font used to render the text
 * @param spacing	The gap between letters, in pixels
 */
unsigned char text_width(unsigned char *string, unsigned char *font, unsigned char spacing);
/**
 * Obtain the height of a string in pixels.  
 *
 * @note At present this will return the height of the font, rather than the
 * specific string.  For example the character 'o' is not as tall as 'L' and
 * does not have the descender of 'g'.
 *
 * @param string	The text to be measured
 * @param font		The font used to render the text
 */
unsigned char text_height(unsigned char *string, unsigned char *font);
/**
 * Draw a line using Bresenham's algorithm.
 *
 * This code credit Tom Ootjers, originally from: http://tinyurl.com/czok7vx
 *
 * @param x1 		The x1 position, from 1 - SCREEN_WIDTH
 * @param x2 		The x2 position, from 1 - SCREEN_WIDTH
 * @param y1 		The y1 position, from 1 - SCREEN_HEIGHT
 * @param y2 		The y2 position, from 1 - SCREEN_HEIGHT
 * @param colour 	0 = OFF, any other value = ON
 */
void draw_line(int x1, int y1, int x2, int y2, char colour);
/**
 * Draw a circle using an efficient circle algorithm.
 *
 * @param centre_x	The x1 position, from 1 - SCREEN_WIDTH
 * @param centre_y	The x2 position, from 1 - SCREEN_WIDTH
 * @param radius	The circle radius, in pixels
 * @param colour 	0 = OFF, any other value = ON
 */
void draw_circle(unsigned char centre_x, unsigned char centre_y, unsigned char radius, unsigned char colour);
/**
 * Draw a filled circle using an efficient circle algorithm.
 *
 * This is separate from draw_circle as it will bring in the draw_line function,
 * which may not be desirable for small code footprint.
 *
 * @param centre_x	The x1 position, from 1 - SCREEN_WIDTH
 * @param centre_y	The x2 position, from 1 - SCREEN_WIDTH
 * @param radius	The circle radius, in pixels
 * @param colour 	0 = OFF, any other value = ON
 */
void draw_filled_circle(unsigned char centre_x, unsigned char centre_y, unsigned char radius, unsigned char colour);

extern void glcd_pixel(unsigned char x, unsigned char y, unsigned char colour);

#endif // _GRAPHICS_H_


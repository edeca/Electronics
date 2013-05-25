/**
 * @file   st7565.h
 * @author David <david@edeca.net>
 * @date   November, 2011
 * @brief  Header for ST7565 graphic LCD library.
 * @sa     <a href="http://XXXXX">ST7565 command reference</a>
 * @sa     <a href="http://edeca.net/wp/electronics/the-st7565-display-controller/">My ST7565 introduction blog</a>
 * @sa     <a href="http://www.ladyada.net/learn/lcd/st7565.html">Adafruit tutorial</a>
 * @details
 *
 * A library for using the ST7565 graphic LCD in serial (software SPI) mode.
 *
 * This library is a low-level interface to the screen only.  Graphics functions (text, images
 * etc) are separate.
 *
 * Each screen requires different settings for brightness (the "volume control" and internal
 * resistor settings).  The initialisation code picks a middle ground, but you may need to 
 * modify this.
 *
 * In serial mode it is not possible to read data back from the screen, meaning we
 * need an array to hold the current data.  For a 128*64 screen, this will require
 * 1KiB of RAM.  We need to know what data is currently on the screen so that we can overlay
 * new pixels onto it, so must keep a copy in local memory.
 * 
 * SPI timings have been checked using the MPLAB Simulator, with Vcc of 3.3v it is impossible 
 * to violate the datasheet guidelines even up to 64Mhz.
 *
 * This code has been tested on a PIC 18F26K20 at 64Mhz using the internal PLL.  No
 * adverse effects were noticed at this speed.  You will need the HiTech delay routines
 * or an equivalent.
 *
 * Example usage (initialisation):
 * @code
 *    // Initialise the screen, turning it on
 *    glcd_init();
 *
 *    // Clear the screen's internal memory 
 *    glcd_blank();
 *
 *    // Set the screen's brightness, if required.  See below for information
 *    // on how this works.
 *    glcd_contrast(3, 25);
 * @endcode
 *
 * Example usage (writing to the screen):
 * @code
 *    // Set some pixels in the RAM buffer
 *    glcd_pixel(1, 1, 1);
 *    glcd_pixel(2, 1, 1);
 *    glcd_pixel(1, 2, 1);
 *    glcd_pixel(2, 2, 1);
 *
 *    // Clear a pixel in the RAM buffer
 *    glcd_pixel(64, 64, 0);
 *
 *    // Copy the RAM buffer to the screen
 *    glcd_refresh();
 * @endcode
 *
 * @note This is a low level library only, with support for setting & clearing pixels.  For text 
 * or graphics functions, please see my graphics library.
 * 
 * This code is released under the BSD license.  Please see BSD-LICENSE.TXT for more information.
 *
 * @todo Check timings compared to datasheet, supply a max recommended Fosc.
 * @todo Handle different sized screens if appropriate, e.g. 128*32 (some code is still
 *       fixed to certain screen/page sizes).
 */
#ifndef _ST7565_H_
#define _ST7565_H_

/** Command: turn the display on */
#define GLCD_CMD_DISPLAY_ON 		0b10101111
/** Command: turn the display off */
#define GLCD_CMD_DISPLAY_OFF 		0b10101110

/** Command: set all points on the screen to normal. */
#define GLCD_CMD_ALL_NORMAL			0b10100100
/** Command: set all points on the screen to "on", without affecting 
             the internal screen buffer. */
#define GLCD_CMD_ALL_ON				0b10100101

/** Command: disable inverse (black pixels on a white background) */
#define GLCD_CMD_DISPLAY_NORMAL		0b10100110
/** Command: inverse the screen (white pixels on a black background) */
#define GLCD_CMD_DISPLAY_REVERSE	0b10100111

/** Command: set LCD bias to 1/9th */
#define GLCD_CMD_BIAS_9				0b10100010
/** Command: set LCD bias to 1/7th */
#define GLCD_CMD_BIAS_7				0b10100011

/** Command: set ADC output direction to normal. */
#define GLCD_CMD_HORIZONTAL_NORMAL	0b10100000
/** Command: set ADC output direction reverse (horizontally flipped). 
             Note that you should use the glcd_flip_screen function so that
             the width is correctly accounted for. */
#define GLCD_CMD_HORIZONTAL_REVERSE	0b10100001

/** Command: set common output scan direction to normal. */
#define GLCD_CMD_VERTICAL_NORMAL	0b11000000
/** Command: set common output scan direction to reversed (vertically flipped). */
#define GLCD_CMD_VERTICAL_REVERSE	0b11001000

/** Command: select the internal power supply operating mode. */
#define GLCD_CMD_POWER_CONTROL		0b00101000

/** Command: set internal R1/R2 resistor bias (OR with 0..7) */
#define GLCD_CMD_RESISTOR			0b00100000
/** Command: enter volume mode, send this then send another command
             byte with the contrast (0..63).  The second command
			 must be sent for the GLCD to exit volume mode. */
#define GLCD_CMD_VOLUME_MODE		0b10000001

#define GLCD_CMD_DISPLAY_START		0b01000000

/** Command: set the least significant 4 bits of the column address. */
#define GLCD_CMD_COLUMN_LOWER		0b00000000
/** Command: set the most significant 4 bits of the column address. */
#define GLCD_CMD_COLUMN_UPPER		0b00010000
/** Command: Set the current page (0..7). */
#define GLCD_CMD_SET_PAGE			0b10110000

/** Command: software reset (note: should be combined with toggling GLCD_RS) */
#define GLCD_CMD_RESET				0b11100010

/** Command: no operation (note: the datasheet suggests sending this periodically
             to keep the data connection alive) */
#define	GLCD_CMD_NOP				0b11100011

/**
 * Initialise the screen.  This should be called first.
 */
void glcd_init();
/**
 * Send a command byte to the screen.  See GLCD_CMD_ constants for
 * a list of commands.
 */
void glcd_command(char);
/** 
 * Send a data byte to the screen. 
 */
void glcd_data(char);
/**
 * Update the screen with the contents of the RAM buffer.
 */
void glcd_refresh();
/**
 * Clear the screen, without affecting the buffer in RAM.
 * 
 * Useful at startup as the memory inside the screen may
 * contain "random" data.
 */
void glcd_blank();
/**
 * Set a single pixel 
 * 
 * @param x 		The x position, from 1 - SCREEN_WIDTH
 * @param y 		The y position, from 1 - SCREEN_HEIGHT
 * @param colour 	0 = OFF, any other value = ON
 */
void glcd_pixel(unsigned char x, unsigned char y, unsigned char colour);
/** 
 * Flip the screen in the alternate direction vertically.
 *
 * Can be used if the screen is mounted in an enclosure upside
 * down.
 */
void glcd_flip_screen(unsigned char flip);
/** 
 * Inverse the screen, swapping "on" and "off" pixels.
 *
 * This does not affect the RAM buffer or the screen memory, the controller
 * is capable of reversing pixels with a single command.
 */
void glcd_inverse_screen(unsigned char inverse);
/** 
 * Fill the local RAM buffer with a test pattern and send it to the screen.
 *
 * Useful for ensuring that the screen is receiving data correctly or for 
 * adjusting contrast.
 */
void glcd_test_card();
/**
 * Set the contrast of the screen.  This involves two steps, setting the 
 * internal resistor ratio (R1:R2) and then the contrast.
 *
 * Tip: Find a resistor ratio that works well with the screen and stick to it
 *      throughout.  Then adjust the contrast dynamically between 0 and 63.
 *
 * @param resistor_ratio	Ratio of the internal resistors, from 0-7
 * @param contrast			Contrast, from 0-63
 */
void glcd_contrast(char resistor_ratio, char contrast);

/** Global variable that tracks whether the screen is the "normal" way up. */
unsigned char glcd_flipped = 0;

#endif // _ST7565_H_

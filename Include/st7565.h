/**
 * @file   st7565.h
 * @author David <david@edeca.net>
 * @date   November, 2011
 * @brief  Header for ST7565 graphic LCD library.
 * @sa     <a href="http://XXXXX">ST7565 command reference</a>
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
 * need a local array to hold the current data.  For a 128*64 screen, this will require
 * 1KiB of RAM.
 *
 * This code has been tested on a PIC 18F26K20 at 64Mhz using the internal PLL.  No
 * adverse effects were noticed at this speed.
 *
 * Example usage (initialisation):
 * @code
 *    // Initialise the screen, turning it on
 *    glcd_init();
 *
 *    // Clear the screen's internal memory 
 *    glcd_blank();
 *
 *    // Set the screen's brightness, if required.
 *    glcd_command(GLCD_CMD_RESISTOR | 0x3)
 *    glcd_command(GLCD_CMD_VOLUME_MODE);
 *    glcd_command(0x0A);
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
 * @todo Check timings compared to datasheet, supply a max recommended Fosc.
 */
#ifndef _ST7565_H_
#define _ST7565_H_

// Setup for ST7565R in SPI mode
/** The chip select pin */
#define GLCD_CS1 LATB0
/** The reset pin (this is required and should not be tied high) */
#define GLCD_RESET LATB1
/** The A0 pin, which selects command or data mode */
#define GLCD_A0 LATB2
/** The clock pin */
#define GLCD_SCL LATB3
/** The data pin */
#define GLCD_SDA LATB4

/** Screen width in pixels (tested with 128) */
#define SCREEN_WIDTH 128
/** Screen height in pixels (tested with 64) */
#define SCREEN_HEIGHT 64

/** Command to turn the display on */
#define GLCD_CMD_DISPLAY_ON 		0b10101111
/** Command to turn the display off */
#define GLCD_CMD_DISPLAY_OFF 		0b10101110

/** Sets all points on the screen to normal. */
#define GLCD_CMD_ALL_NORMAL			0b10100100
/** Sets all points on the screen to "on", without affecting the internal screen buffer. */
#define GLCD_CMD_ALL_ON				0b10100101

// Sets LCD display normal or reverse ????
#define GLCD_CMD_DISPLAY_NORMAL		0b10100110
#define GLCD_CMD_DISPLAY_REVERSE	0b10100111

#define GLCD_CMD_ADC_NORMAL			0b10100000
#define GLCD_CMD_ADC_REVERSE		0b10100001

#define GLCD_CMD_BIAS_9				0b10100010
#define GLCD_CMD_BIAS_7				0b10100011

// COM output scan direction, normal or reversed.
#define GLCD_CMD_OUTPUT_NORMAL		0b11000000
#define GLCD_CMD_OUTPUT_REVERSE		0b11001000


#define GLCD_CMD_POWER_CONTROL		0b00101000

#define GLCD_CMD_RESISTOR			0b00100000
#define GLCD_CMD_VOLUME_MODE		0b10000001

#define GLCD_CMD_DISPLAY_START		0b01000000

#define GLCD_CMD_COLUMN_LOWER		0b00000000
#define GLCD_CMD_COLUMN_UPPER		0b00010000
#define GLCD_CMD_SET_PAGE			0b10110000

/** Command: software reset (note: should be combined with toggling GLCD_RS) */
#define GLCD_CMD_RESET				0b11100010

/** Command: no operation (note: the datasheet suggests sending this periodically
             to keep the data connection alive) */
#define	GLCD_CMD_NOP				0b11100011

// Note that read-modify-write mode is not useful with SPI, as it is not possible
// to read.  Therefore this command is not included.

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

/** Buffer to hold the current screen contents. */
unsigned char glcd_buffer[SCREEN_WIDTH * SCREEN_HEIGHT];

#endif // _ST7565_H_

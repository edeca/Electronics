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

/** Define this if your screen is incorrectly shifted by 4 pixels */
#define ST7565_REVERSE

/** By default we only write pages that have changed.  Undefine this
    if you want less/faster code at the expense of more SPI operations. */
#undef ST7565_DIRTY_PAGES 1
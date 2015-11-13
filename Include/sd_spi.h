/**
 * @file    sd_spi.h
 * @author  David <david@edeca.net>
 * @date    November, 2015
 * @brief   Header for SD card library
 * @copyright BSD license, see BSD-LICENSE.txt
 * @details
 *
 * A library for accessing SD cards, using the SPI protocol.
 *
 * This code works with SDSC (standard capacity), SDHC (high capacity) and
 * SDXC (extended capacity) cards.  It does *not* support MMC cards and there
 * are no plans to implement this.
 * 
 * It has been tested with a number of real cards of varying capacities to 
 * identify cases where retail branded cards misbehave.  See code comments for
 * workarounds which have been introduced to address these.  Workarounds may
 * be enabled or disabled with a single #define.
 * 
 * References in the code to SD specifications relate to the document:
 * 
 *  "Part 1 - Physical Layer Simplified Specification Version 4.10
 *     January 22, 2013" (copyright SD Group)
 * 
 * Code using this library must provide a number of functions that implement 
 * SPI.  This allows the code to be used with either bit-banged or peripheral 
 * SPI.  There is currently no efficient support for DMA, all calls to SPI 
 * functions are byte width for both arguments and return values.
 * 
 * Features specific to this library to reduce code size include:
 *
 *   - block reads / writes are always 512 bytes, including for cards that
 *     support smaller or larger sizes.
 * 
 *   - misaligned reads / writes are not supported.  All reads / writes will
 *     begin and end on a block boundary (multiples of 512).
 * 
 *   - only one card is supported at a time, due to the usage of a global
 *     buffer for card information.  This could be changed at the expense
 *     of additional global variables.
 * 
 * Limitations include:
 * 
 *   - the library blocks, it may be rewritten as interrupt driven code when
 *     all basic SD functions work reliably.
 * 
 *   - the library is not re-entrant due to usage of global stage variable.  Do 
 *     not call it from an interrupt handler.
 */

#define R1 1
#define R1B 1
#define R2 2
#define R3 5

// R1 + 4 bytes
#define R7 5

/**
 * There are no valid SD commands > 127, so we use the MSB to indicate that 
 * the card should not be deselected by _sd_command().
 * 
 * This is used to setup block reads without toggling the chip select in the 
 * middle, which is not valid and is rejected by some (but not all) cards.
 */
#define SD_KEEP_CARD_SELECTED 0x80

// MSB is not used in the status register, so we use it to
// indicate a hardware error
#define SD_ERROR_TIMEOUT 0x80

#define SD_CSD_VERSION_1 0
#define SD_CSD_VERSION_2 1

#define SD_BLOCKSIZE 512

#define MSK_IDLE 0x01
#define MSK_ERASE_RST 0x02
#define MSK_ILL_CMD 0x04
#define MSK_CRC_ERR 0x08
#define MSK_ERASE_SEQ_ERR 0x10
#define MSK_ADDR_ERR 0x20
#define MSK_PARAM_ERR 0x40

// This token precedes any block reads and writes
// See section 7.3.3.2 of the simplified specification.
#define SD_TOKEN_START_BLOCK 0xFE

/*#define SD_TOK_WRITE_STARTBLOCK 0xFE
#define SD_TOK_READ_STARTBLOCK_M 0xFE
#define SD_TOK_WRITE_STARTBLOCK_M 0xFC
#define SD_TOK_STOP_MULTI 0xFD*/

/* Error token is 111XXXXX */
#define MSK_TOK_DATAERROR 0xE0
/* Bit fields */
#define MSK_TOK_ERROR 0x01
#define MSK_TOK_CC_ERROR 0x02
#define MSK_TOK_ECC_FAILED 0x04
#define MSK_TOK_CC_OUTOFRANGE 0x08
#define MSK_TOK_CC_LOCKED 0x10
/* Mask off the bits in the OCR corresponding to voltage range 3.2V to
* 3.4V, OCR bits 20 and 21 */
#define MSK_OCR_33 0xC0
/* Number of times to retry the probe cycle during initialization */
#define SD_INIT_TRY 50
/* Number of tries to wait for the card to go idle during initialization */
#define SD_IDLE_WAIT 100

//#define SD_CMD_TIMEOUT 32
#define SD_CMD_TIMEOUT 128
/******************************** Basic command set **************************/
/* Reset cards to idle state */
#define CMD0 0
#define CMD0_R R1
/* Read the OCR (MMC mode, do not use for SD cards) */
#define CMD1 1
#define CMD1_R R1

/* Ask card if it can operate in supplied voltage range */
/* Arguments: host voltage, only one bit may be set at a time */
/*            check pattern can be any 8 bit pattern */
#define CMD8 8
#define CMD8_R R7

/* Card sends the CSD */
#define CMD9 9
#define CMD9_R R1
/* Card sends CID */
#define CMD10 10
#define CMD10_R R1
/* Stop a multiple block (stream) read/write operation */
#define CMD12 12
#define CMD12_R R1B
/* Get the addressed card's status register */
#define CMD13 13
#define CMD13_R R2

/* Block read commands */

#define CMD16 16
#define CMD16_R R1
/* Read a single block */
#define CMD17 17
#define CMD17_R R1

/* Read multiple blocks until a CMD12 */
#define CMD18 18
#define CMD18_R R1


/***************************** Block write commands *************************/
/* Write a block of the size selected with CMD16 */
#define CMD24 24
#define CMD24_R R1
/* Multiple block write until a CMD12 */
#define CMD25 25
#define CMD25_R R1
/* Program the programmable bits of the CSD */
#define CMD27 27
#define CMD27_R R1
/***************************** Write protection *****************************/
/* Set the write protection bit of the addressed group */
#define CMD28 28
#define CMD28_R R1B
/* Clear the write protection bit of the addressed group */
#define CMD29 29
#define CMD29_R R1B
/* Ask the card for the status of the write protection bits */
#define CMD30 30
#define CMD30_R R1
/***************************** Erase commands *******************************/
/* Set the address of the first write block to be erased */
#define CMD32 32
#define CMD32_R R1
/* Set the address of the last write block to be erased */
#define CMD33 33
#define CMD33_R R1
/* Erase the selected write blocks */
#define CMD38 38
#define CMD38_R R1B
/***************************** Lock Card commands ***************************/
/* Commands from 42 to 54, not defined here */
/***************************** Application-specific commands ****************/
/* Flag that the next command is application-specific */
#define CMD55 55
#define CMD55_R R1
/* General purpose I/O for application-specific commands */
#define CMD56 56
#define CMD56_R R1
/* Read the OCR (SPI mode only) */
#define CMD58 58
#define CMD58_R R3
/* Turn CRC on or off */
#define CMD59 59
#define CMD59_R R1
/***************************** Application-specific commands ***************/
/* Get the SD card's status */
#define ACMD13 13
#define ACMD13_R R2
/* Get the number of written write blocks (Minus errors ) */

#define ACMD22 22
#define ACMD22_R R1
/* Set the number of write blocks to be pre-erased before writing */
#define ACMD23 23
#define ACMD23_R R1
/* Get the card's OCR (SD mode) */
#define ACMD41 41
#define ACMD41_R R1
/* Connect or disconnect the 50kOhm internal pull-up on CD/DAT[3] */
#define ACMD42 42
#define ACMD42_R R1
/* Get the SD configuration register */
#define ACMD51 42
#define ACMD51_R R1

/** SD standard capacity card type */
#define SD_TYPE_SDSC 0
/** SD high capacity card type */
#define SD_TYPE_SDHC 1
/** SD extended capacity card type */
#define SD_TYPE_SDXC 2

/**
 * Represents a raw CID register as returned by the card.  A more friendly
 * version is available as sd_card_info by calling sd_get_card_data().
 */
typedef union sd_cid_register {
    uint8_t data[16];
    struct _values {
        uint8_t manufacturer_id;
        unsigned char oem_id[2];
        unsigned char product_name[5];
        uint8_t revision;
        uint32_t serial_number;
        uint16_t manufacture_date;
        uint8_t crc7;
    } values;

} sd_cid_register_t;

/**
 * A code friendly version of the CID register.  This can be populated by
 * calling sd_get_card_data() with an empty structure.
 */
typedef struct sd_card_info {
    uint8_t manufacturer_id;    /** Card manufacturer ID */
    unsigned char oem_id[3];    /** OEM ID, two character string */
    unsigned char product_name[6]; /** Product name, five character string */
    uint8_t revision_major;     /** Card revision, major byte */
    uint8_t revision_minor;     /** Card revision, minor byte */
    uint32_t serial_number;     /** Card serial number */
    uint8_t manufacture_month;  /** Month of manufacture */
    uint16_t manufacture_year;  /** Year of manufacture */
} sd_card_info_t;

/**
 * Card data used for internal housekeeping.
 */
typedef struct sd_card_data {
    uint8_t csd_version;      /** Version of card (1 or 2 supported) */
    /*
    uint8_t taac;
    uint8_t nsac;
    uint8_t tran_speed;
    uint16_t command_classes;
    uint8_t read_block_length;
    uint8_t read_block_partial;
    /*
    uint8_t dsr_implemented;
    */
    uint32_t total_blocks;    /**< Card size, in multiples of 512 byte blocks */
} sd_card_data_t;

/**
 * @name Public functions
 * 
 * These are the public functions called to achieve basic interaction with an
 * SD card.
 * 
 * @{
 */
/**
 * Initialise an SD card and read essential registers to update internal status.
 * 
 * If this returns 1 then you can switch SPI to high speed mode.
 * 
 * @return 0 for failure (see last error), 1 for success
 */
uint8_t sd_initialize();
void sd_pack_argument(uint8_t *argument, uint32_t value);
uint16_t sd_read_register(uint8_t reg, uint8_t *buffer);

uint16_t sd_read_block(uint32_t address, uint8_t *buffer);
/**
 * Get data such as the card manufacturer, product name and serial number in
 * an easy to manage structure.  This reads the CID register from the card and
 * converts all data from the packed format to real integer / string values.
 * 
 * @param info An empty sd_card_info_t structure
 * @return 0 for failure (see last error), 1 for success
 */
uint8_t sd_get_card_data(sd_card_info_t info);
/** 
 * Get the size of the current card.
 * 
 * @return Card size in kibibytes
 */
uint32_t sd_get_card_kib();
/** @} */

/**
 * @name Internal functions
 * 
 * These internal functions can be called directly if you know what you are
 * doing, but some may alter internal state or place the card into an undefined
 * state.
 * 
 * @{
 */
/** 
 * Get basic card size details by reading the CSD register.  Mandatory, called
 * during initialisation.
 * 
 * @return 0 for failure (see last error), 1 for success
 */
uint8_t _sd_read_csd();
/**
 * Send a raw command to the card.
 * 
 * If MSB of cmd is set then the SPI CS line will not be deasserted.  This is 
 * essential for setting up block read / writes - caller is responsible for 
 * deselecting the card when the block transaction is finished.
 * 
 * @param cmd command identifier
 * @param argument packed argument data on input, card response copied to this variable
 * @param response_len length of response to read
 * @return 0 for failure (see last error), 1 for success 
 */
uint8_t _sd_command(uint8_t cmd, uint8_t *argument, uint8_t response_len);
/**
 * Read from an SD card.  Used to obtain both registers and user data.
 * 
 * The read must have been setup with the appropriate command before calling
 * this function.  When calling _sd_command() remember to set the MSB (e.g. 
 * using SD_KEEP_CARD_SELECTED) to initiate the transaction.
 * 
 * @param dest destination buffer to read data into
 * @param count the number of bytes to read (register length or 512 for data)
 * @return 0 for error, or number of bytes read (should equal count on success)
 */
uint16_t _sd_read(uint8_t *dest, uint16_t count);
/**
 * Implements SD CRC7 algorithm to verify outgoing and incoming SPI data.
 * 
 * This is only included if CRC support is enabled.
 * 
 * @param crc  current CRC byte (or zero on first call)
 * @param data data byte to update CRC
 * @return new CRC byte
 */
uint8_t _sd_crc7(uint8_t crc, uint8_t data);
/**
 * Wraps everything that needs to happen at the start of an SPI transaction.
 */
void inline _sd_start();
/**
 * Wraps everything that needs to happen at the end of an SPI transaction.
 */
void inline _sd_stop();
/** @} */

/**
 * @name External functions
 * 
 * These functions need to be provided and fully implemented by the code 
 * using this library.  This allows portability to various hardware or
 * software SPI implementations.
 * 
 * @{
 */
/**
 * Send a byte on the SPI bus to the selected card.  The card will have been
 * selected by the library using spi_select_card().
 * 
 * @ingroup sd_external
 * @param data byte to be sent to SD card (on MOSI line)
 * @return the byte read from SD card (on MISO line)
 */
extern unsigned char spi_byte(uint8_t data);
/**
 * A function that will idle the SPI bus for a given number of cycles.  The byte 
 * sent by SPI should be 0xFF.  Most implementations should simply wrap spi_byte
 * with the correct arguments.
 * 
 * @param cycles Number of idle bytes to send
 */
extern void inline spi_idle(uint8_t cycles);
/**
 * A function that will select the card by setting the SPI CS line correctly 
 * (driven active low).
 */
extern void inline spi_select_card();
/**
 * A function that will deselect the card by setting the SPI CS line correctly 
 * (not driven, allowing pull-up resistor to bring it high).
 */
extern void inline spi_deselect_card();
/** @} */

/**
 * @name Configuration directives
 * 
 * These directives control what features of the library are enabled.  You can
 * override defaults by defining the desired value before including this file.
 * 
 * @{
 */
/** Configuration: enable CRC for all transactions.  Disabled by default. */
#ifndef SD_CONFIG_CRC
#define SD_CONFIG_CRC 0
#endif

/** Configuration: Work around common problems that aren't strictly standards 
 *  compliant but appeared during real world testing of this library.
 *  Enabled by default */
#ifndef SD_CONFIG_WORKAROUNDS
#define SD_CONFIG_WORKAROUNDS 1
#endif
/** @} */

/** Global variable: card data used throughout library */
sd_card_data_t card_data;
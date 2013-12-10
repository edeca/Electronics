// Replace with whatever header provides your pin definitions
#include <xc.h>
#include <string.h>
#include <stdint.h>
#include "sd_spi.h"
// TODO: Consider how to do delays nicely in this library
#include "delay.h"

// DEBUGGING ONLY - printf in here
#include <stdio.h>

// TODO: Return a good/bad value
//
// Setup the block read with the appropriate command, then read the 16 byte block
uint16_t sd_read_register(uint8_t cmd, uint8_t *buffer) {
    memset(buffer, 0, 16);
    // We can reuse the output buffer here, both CMD9 and CMD10 take stuff bits
    // that can be any value.
    cmd = cmd | SD_KEEP_CARD_SELECTED;
    sd_command(cmd, buffer, R1);
    
    // TODO: Check we get the right amount of bytes back, or error
    uint16_t bytes_read = sd_block_read(buffer, 16);
    // TODO: Check CRC instead of fake 2 byte read
    // TODO: Deselect card before 2 byte extra?
    spi_idle(4);
    sd_stop();
    
    // TODO - We could just return 1/0 here by checking bytes_read == 16, so
    // long as this function is never used for lengths <> 16
    return bytes_read;
}

// Fixed 512 byte read (suitable for SDSC/HC/XC)
uint16_t sd_read_block(uint32_t address, uint8_t *buffer) {
    uint8_t arg[4];
    sd_pack_argument(arg, address);
    sd_command(CMD17 | SD_KEEP_CARD_SELECTED, arg, CMD17_R);

    // TODO: Check we get the right amount of bytes back, or error
    uint16_t bytes_read = sd_block_read(buffer, 512);
    // TODO: Check CRC instead of fake 2 byte read
    // TODO: Deselect card before 2 byte extra?
    spi_idle(4);
    sd_stop();

    printf("in sd_read_block, got %u bytes back from card\r\n", bytes_read);

    // TODO - We could just return 1/0 here by checking bytes_read == 16, so
    // long as this function is never used for lengths <> 16
    return bytes_read;
}

#ifdef SD_CRC_ENABLED
uint8_t sd_crc7(uint8_t crc, uint8_t data) {
    uint8_t len = 8;
    uint8_t tmp;

    while (len) {
        tmp = crc & 0x40;
        if (data & 0x80)
            tmp ^= 0x40;

        data <<= 1;
        crc <<= 1;

        if (tmp)
            crc ^= 0x09;

        len--;
    }

    return crc;
}
#endif

// This can be used for all commands as all response
// types start with the same status byte
//
// If MSB of cmd is set then it will not deassert the card.  This is good
// for setting up block reads - but the caller is responsible for deselecting
// the card.
uint8_t sd_command(unsigned char cmd, unsigned char *argument, uint8_t response_len) {
    uint8_t status = 0;
    uint8_t crc = 0;
    uint8_t timeout = SD_CMD_TIMEOUT;
    uint8_t cmd_byte = (cmd & 0x7F) | 0x40;

    //printf("Sending command CMD%02u\r\n", cmd);
    sd_start();

#ifdef SD_CRC_ENABLED
    crc = sd_crc7(crc, cmd_byte);
    crc = sd_crc7(crc, argument[3]);
    crc = sd_crc7(crc, argument[2]);
    crc = sd_crc7(crc, argument[1]);
    crc = sd_crc7(crc, argument[0]);
    crc <<= 1;
    crc |= 1;
#else
    // If CRC isn't enabled then we need to manually calculate the correct
    // values for CMD0 and CMD8, which always require a valid CRC.
    if (cmd_byte == 0x40)
        crc = 0x95;
    else if (cmd_byte == 0x48)
        // This CRC value for CMD8 is only valid with the default arguments
        // sent by sd_initialise().
        crc = 0x69;
#endif

    // Send the command
    spi_byte(cmd_byte);

    // TODO - see if passing a null pointer for "no argument" and branching
    // on it results in more efficient code.
    // Send the argument
    spi_byte(argument[3]);
    spi_byte(argument[2]);
    spi_byte(argument[1]);
    spi_byte(argument[0]);

    // Send CRC
    spi_byte(crc);

    // Wait until the SD card is ready, by checking the MSB of the byte
    // received.  This  will always be 0 in a valid R1 response.
    do {
        status = spi_byte(0xFF);
        timeout--;
    } while ((status & 0x80) && timeout);

    // TODO: Set last error here in a global sd status struct
    if (timeout == 0) return SD_ERROR_TIMEOUT;

    // DEBUG
    //if (status & MSK_CRC_ERR) printf(" - CRC error!\r\n");

    while (--response_len) {
        *argument++ = spi_byte(0xFF);
    }

    // Edit: Check the todo still but leaving this in here messes up a block
    // read for CID to a 4GB kingston card.
    // If this is required we will need to invent a "start transaction" and
    // "stop transaction" macro that idle the bus for 2 cycles before/after
    // command sequences.
    // 4GB SanDisk card absolutely needs this in order to operate.  Odd, or the
    // XC8 compiler is doing unusual things.
    if ((cmd & SD_KEEP_CARD_SELECTED) == 0) {
        sd_stop();
    }

    // We always return the status byte (equivalent to R1)
    return status;
}

void inline sd_stop() {
    // See section 4.4 of the standard, at least 8 clocks (2 bytes) must be
    // sent after the last command.
    //
    // See also: http://elm-chan.org/docs/mmc/mmc_e.html which shows why extra
    // clocks are necessary, in order to release the SPI bus.
    SD_CS = 1;
    spi_idle(2);
}

void inline sd_start() {
    SD_CS = 0;
}

// Returns the number of bytes read, or 0 on error.
uint16_t sd_block_read(uint8_t *dest, uint16_t count) {
    uint16_t bytes_read = 0;
    uint8_t timeout = SD_CMD_TIMEOUT;

    // Wait for the start of block token with timeout.
    // When tested on a 4GB Sandisk card there was an extra FF byte between R1
    // and 0xFE token.
    do {
        timeout--;
        // TODO: Debug value below, work out a proper value
        DelayMs(100);
    } while (spi_byte(0xFF) != SD_TOKEN_START_BLOCK && timeout);

    if (timeout == 0) return 0;

    while (count--) {
        *dest++ = spi_byte(0xFF);
        bytes_read++;
    }

    // TODO: Read two more bytes (CRC16) and check them

    return bytes_read;
}

// Returns: 0 for failure (see the last error field), 1 for success
// If this returns 1 then you can switch SPI implementation to high speed mode

uint8_t sd_initialize() {
    unsigned char status;
    uint8_t timeout;
    // Used for request arguments and response data
    unsigned char data[5] = {0, 0, 0, 0, 0};

    // Send a minimum of 74 clock cycles to allow the SD card to
    // initialise itself.
    sd_start();
    spi_idle(100);
    sd_stop();

    // CMD0
    // @todo Bail on timeouts, they aren't good for us
    status = sd_command(CMD0, data, CMD0_R);
    if (status == SD_ERROR_TIMEOUT) {
        // TODO: Set last error here
        return 0;
    }

    // Send CMD8, if it's illegal then v1 card, if not v2.  Mandatory as of
    // SPI specification v2.  We send a VHS of 1 (2.7-3.6v) and check pattern
    // of 0xA5 (this can be any value).
    // CMD8 *always needs correct CRC*
    sd_pack_argument(data, 0x1A5);
    status = sd_command(CMD8, data, CMD8_R);

    if (status & MSK_ILL_CMD) {
        // Version 1 initialisation
        // TODO: Set v1 card in struct
        //printf("CMD8 illegal, v1 card\r\n");
    } else {
        // Version 2 initialisation
        // TODO: Set v2 card in struct
        // TODO: Is there anything else in CMD8 response that we need to check?

        if (data[2] != 0x01) {
            // TODO: Set error and return
            //printf("voltage range unsupported\r\n");
            return 0;
        }

        // @todo Set error and return
        if (data[3] != 0xA5) {
            // TODO: Set error and return
            //printf("check pattern does not match\r\n");
            return 0;
        }
    }

    timeout = 255;

    // Check that the operating voltage is supported by the card by reading
    // the OCR.
    // @todo Check voltage range is legal for the system Vdd
    // @todo Check for illegal command, which indicates "not an SD card"
    status = sd_command(CMD58, data, CMD58_R);
    //printf("Status from CMD58: %02x\r\n", status);

    // ACMD41 to start initialisation, wait for it to finish or timeout.
    // Set bit 30 (HCS) to indicate we support SDHC/SDXC.
    sd_pack_argument(data, 0x40000000);
    do {
        // Arguments to CMD55 can be "stuff bits" rather than reserved bits, so
        // use same data as ACMD41.
        sd_command(CMD55, data, CMD55_R);
        status = sd_command(ACMD41, data, ACMD41_R) & MSK_IDLE;
        DelayMs(1);
        timeout--;
    } while (status && timeout);

    // Timeout whilst waiting for the initialisation process to complete
    if (timeout == 0) {
        // Set last error to SD_ERROR_TIMEOUT
        return 0;
    }

    return 1;
}

void sd_pack_argument(uint8_t *argument, uint32_t value) {
    argument[0] = (uint8_t) value;
    value >>= 8;
    argument[1] = (uint8_t) value;
    value >>= 8;
    argument[2] = (uint8_t) value;
    value >>= 8;
    argument[3] = (uint8_t) value;
}

unsigned char sd_set_blocklen(unsigned long length, unsigned char *response) {
    unsigned char argument[4];
    sd_pack_argument(argument, length);
    return (sd_command(CMD16, argument, CMD16_R));
}
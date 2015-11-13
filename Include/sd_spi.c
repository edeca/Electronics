// Replace with whatever header provides your pin definitions
#include <xc.h>
#include <string.h>
#include <stdint.h>
#include "sd_spi.h"

/* @todo Consider how to do delays in this library, use extern? */
#include "delay.h"

// DEBUGGING ONLY - printf in here
#include <stdio.h>

#if SD_CONFIG_CRC==1
#warning sd_spi.c: SD CRC support is enabled
#else
#warning sd_spi.c: SD CRC support is disabled
#endif

#if SD_CONFIG_WORKAROUNDS==1
#warning sd_spi.c: SD workarounds are enabled
#else
#warning sd_spi.c: SD workarouns are disabled
#endif

// TODO: Return a good/bad value
//
// Setup the block read with the appropriate command, then read the 16 byte block
uint16_t sd_read_register(uint8_t cmd, uint8_t *buffer) {
    uint16_t bytes_read;
    
    memset(buffer, 0, 16);
    
    // We can reuse the output buffer here, both CMD9 and CMD10 take stuff bits
    // that can be any value.
    cmd = cmd | SD_KEEP_CARD_SELECTED;
    _sd_command(cmd, buffer, R1);
    
    // TODO: Check we get the right amount of bytes back, or error
    bytes_read = _sd_read(buffer, 16);
    // TODO: Check CRC instead of fake 2 byte read
    // TODO: Deselect card before 2 byte extra?
    spi_idle(4);
    _sd_stop();
    
    // TODO - We could just return 1/0 here by checking bytes_read == 16, so
    // long as this function is never used for lengths <> 16
    return bytes_read;
}

// Fixed 512 byte read (suitable for SDSC/HC/XC)
// address - block address for card (block, *not* byte address)
// buffer - pointer to a preallocated 512 byte buffer
uint16_t sd_read_block(uint32_t block_num, uint8_t *buffer) {
    uint8_t arg[4];
    
    /* @todo If card is SDHC, address should be multiplied by 512, these cards
     *       are byte addressed */
    if (card_data.csd_version == SD_CSD_VERSION_1) {
        block_num *= 512;
    }
            
    sd_pack_argument(arg, block_num);
    _sd_command(CMD17 | SD_KEEP_CARD_SELECTED, arg, CMD17_R);

    // TODO: Check we get the right amount of bytes back, or error
    uint16_t bytes_read = _sd_read(buffer, 512);
    // TODO: Check CRC instead of fake 2 byte read
    // TODO: Deselect card before 2 byte extra?
    spi_idle(4);
    _sd_stop();

    printf("got %u bytes\r\n", bytes_read);
    return bytes_read;
}

#if SD_CONFIG_CRC==1
uint8_t _sd_crc7(uint8_t crc, uint8_t data) {
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
uint8_t _sd_command(unsigned char cmd, unsigned char *argument, uint8_t response_len) {
    uint8_t status = 0;
    uint8_t crc = 0;
    uint8_t timeout = SD_CMD_TIMEOUT;
    uint8_t cmd_byte = (cmd & 0x7F) | 0x40;

    //printf("Sending command CMD%02u\r\n", cmd);
    _sd_start();

#if SD_CONFIG_CRC==1
    crc = _sd_crc7(crc, cmd_byte);
    crc = _sd_crc7(crc, argument[3]);
    crc = _sd_crc7(crc, argument[2]);
    crc = _sd_crc7(crc, argument[1]);
    crc = _sd_crc7(crc, argument[0]);
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

#if SD_CONFIG_CRC==1
    /* @todo If using CRC, flag errors here */
    //if (status & MSK_CRC_ERR) printf(" - CRC error!\r\n");
#endif
    
    while (--response_len) {
        *argument++ = spi_byte(0xFF);
    }

    // See notes for sd_stop, unless we are setting up a block read/write
    // then we need to deselect the card here.
    if ((cmd & SD_KEEP_CARD_SELECTED) == 0) {
        _sd_stop();
    }

    // We always return the status byte (equivalent to R1)
    return status;
}

void inline _sd_stop() {
    // See section 4.4 of the standard, at least 8 clocks (2 bytes) must be
    // sent after the last command.
    //
    // See also: http://elm-chan.org/docs/mmc/mmc_e.html which shows why extra
    // clocks are necessary, in order to release the SPI bus.
    spi_deselect_card();
    spi_idle(2);
}

void inline _sd_start() {
    spi_select_card();
}

uint16_t _sd_read(uint8_t *dest, uint16_t count) {
    uint16_t bytes_read = 0;
    uint8_t timeout = SD_CMD_TIMEOUT;

    // Wait for the start of block token with timeout.
    // When tested on a 4GB Sandisk card there was an extra FF byte between R1
    // and 0xFE token.
    do {
        timeout--;
        // @todo Remove XC8 delays here - check spec for total timeout allowed4
        // Specification section 4.6.2.1 suggests 100mS timeout (minimum) for
        // read operations.
        DelayMs(10);
    } while (spi_byte(0xFF) != SD_TOKEN_START_BLOCK && timeout);

    if (timeout == 0) {
        printf("Timed out during block read?\r\n");
        return 0;
    }

    while (count--) {
        *dest++ = spi_byte(0xFF);
        bytes_read++;
    }

#if SD_CONFIG_CRC==1
    /* @todo If using CRC, read two more bytes (CRC16) and check them */
#endif
    
    return bytes_read;
}

uint8_t sd_initialize() {
    unsigned char status;
    uint8_t timeout;
    // Used for request arguments and response data
    unsigned char data[5] = {0, 0, 0, 0, 0};

    // Send a minimum of 74 clock cycles to allow the SD card to
    // initialise itself.  See specification section 6.4.1.1.
    _sd_start();
    spi_idle(100);
    _sd_stop();

    // CMD0
    // @todo Bail on timeouts, they aren't good for us
    status = _sd_command(CMD0, data, CMD0_R);
    if (status == SD_ERROR_TIMEOUT) {
        // TODO: Set last error here
        return 0;
    }

    // Send CMD8, if it's illegal then v1 card, if not v2.  Mandatory as of
    // SPI specification v2.  We send a VHS of 1 (2.7-3.6v) and check pattern
    // of 0xA5 (this can be any value).
    // CMD8 *always needs correct CRC*
    sd_pack_argument(data, 0x1A5);
    status = _sd_command(CMD8, data, CMD8_R);
    //printf("Status from CMD8 == %u\r\n", status);
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

    timeout = 100;

    // Check that the operating voltage is supported by the card by reading
    // the OCR.
    // @todo Check voltage range is legal for the system Vdd
    // @todo Check for illegal command, which indicates "not an SD card"
    status = _sd_command(CMD58, data, CMD58_R);

    /* Use ACMD41 to start initialisation, wait for it to finish or timeout.
     *
     * According to section 4.2.3 of the spec:
     * "initialization shall be completed within 1 second from the first ACMD41"
     */
    // Set bit 30 (HCS) to indicate we support SDHC/SDXC.
    sd_pack_argument(data, 0x40000000);
    do {
        // Arguments to CMD55 can be "stuff bits" rather than reserved bits, so
        // use same data as ACMD41.
        _sd_command(CMD55, data, CMD55_R);
        status = _sd_command(ACMD41, data, ACMD41_R);

#if SD_CONFIG_WORKAROUNDS==1
        /* Workaround: 
         * 
         * Some cards reject ACMD41 with "illegal command" if the SDHC/SDXC
         * bit is set, despite the specification saying:
         * 
         * "HCS is ignored by the card, which didn't accept CMD8. Standard 
         * Capacity SD Memory Card ignores HCS."
         * 
         * There does not appear to be a way to check HCS support from the card 
         * until after initialisation, so don't advertise it to broken cards.
         */
        if (status & MSK_ILL_CMD) {
            data[3] = 0;
        }
#endif
       
        // @todo Remove XC8 delays here - check spec for total timeout allowed
        // Specification section 4.2.3:
        // "The host repeatedly issues ACMD41 for at least 1 second"
        DelayMs(10);
        timeout--;
        status &= MSK_IDLE;
    } while (status && timeout);

    // Timeout whilst waiting for the initialisation process to complete
    if (timeout == 0) {
        printf("Timed out waiting for init\r\n");
        /* @todo set last error to SD_ERROR_TIMEOUT */
        return 0;
    }

    // Read registers to update internal status
    _sd_read_csd();
    
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

uint8_t _sd_read_csd() {
    uint8_t data[16] = { 0 };
    uint32_t c_size, block_nr;
    uint16_t block_len;
    uint8_t c_size_mult, read_bl_len;
    
    // @todo Error handling if this fails
    sd_read_register(CMD9, data);

    // Set the CSD version, used throughout the library to handle the small
    // differences between v1 (SDSC) and v2 (SDHC/SDXC) cards.
    card_data.csd_version = (data[0] >> 6);

    // The card is a SDSC (standard capacity) card
    if (card_data.csd_version == SD_CSD_VERSION_1) {

        // Shift around 12 bits of C_SIZE register
        c_size = data[6] & 0x03;
        c_size <<= 10;
        c_size |= data[7] << 2;
        c_size |= (data[8] & 0xC0) >> 6;

        // Shift around the 3 bits of C_SIZE_MULT
        c_size_mult = (data[9] & 0x3) << 1;
        c_size_mult |= (data[10] & 0x80) >> 7;
        read_bl_len = data[5] & 0xF;

        // block_nr = (c_size + 1) * 2^(c_size_mult + 2)
        block_nr = (c_size + 1) * (2 << c_size_mult + 1);
        // block_len = 2^read_bl_len
        block_len = 2 << (read_bl_len - 1);
        
        //printf("c_size is: %lu\r\n", c_size);
        //printf("c_size_mult is: %u\r\n", c_size_mult);
        //printf("read_bl_len is: %u\r\n", read_bl_len);

        // Calculate capacity in bytes and divide for blocks
        c_size = (block_nr * block_len) / 512;
        card_data.total_blocks = c_size;

    } else if (card_data.csd_version == SD_CSD_VERSION_2) {
        // Shift around the 22 bits of C_SIZE, which can be used directly
        // as the number of 512 byte blocks (sector size is fixed for SDHC/XC)
        //
        // The XC8 compiler has occasional issues with struct members and
        // optimisation, so use a temporary variable.
        c_size = data[7] & 0x3F;
        c_size <<= 16;
        c_size |= data[8] << 8;
        c_size |= data[9];

        // Internally we store the number of 512 byte blocks, C_SIZE gives
        // the number of 512KB blocks so multiply by 1024.
        c_size <<= 10;
        
        card_data.total_blocks = c_size;
    }

    return 1;
}

uint32_t sd_get_card_kib() {
    uint32_t ret = 0;
    ret = (card_data.total_blocks + 1) / 2;
    return ret;
}

uint8_t sd_get_card_data(sd_card_info_t info) {
    sd_cid_register_t cid;

    // Read the CID register
    // @todo Check status here and return error if appropriate
    sd_read_register(CMD10, cid.data);

    memset(info.product_name, 0, 6);
    memset(info.oem_id, 0, 3);

    info.manufacturer_id = cid.values.manufacturer_id;

    strncpy(info.product_name, cid.values.product_name, 5);
    strncpy(info.oem_id, cid.values.oem_id, 2);

    info.revision_minor = cid.values.revision & 0xF;
    info.revision_major = cid.values.revision >> 4;

    info.manufacture_month = cid.data[14] & 0xF;
    info.manufacture_year = cid.data[14] >> 4;
    info.manufacture_year |= cid.data[13] >> 4;
    info.manufacture_year += 2000;

    info.serial_number = cid.values.serial_number;
    
    return 1;
}
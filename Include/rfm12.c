//#include <pic18.h>
#include <htc.h>
#include "rfm12.h"

/// ### DEBUG ONLY ###
#include <stdio.h>

/********
 * RFM12 base library
 * Adapted from Hope RFM12 programming guide
 *
 * David Cannings <david@edeca.net>
 *
 * To use:
 *   - Setup RFM12_ pin definitions
 *   - Implement the RFM interrupt callback (software polling or preferably interrupt based)
 */

void
spi_Init(void) {
    // Chip select line is an output, is active low
    // so start disabled
    RFM12_CS_TRIS = 0;
    RFM12_CS = 1;

    // Setup SPI clock
    RFM12_SCK_TRIS = 0;
    RFM12_SCK = 0;

    RFM12_SDO_TRIS = 0;
    RFM12_SDO = 0;

    // RFM12_SDI and ^IRQ are inputs
    RFM12_SDI_TRIS = 1;
    RFM12_IRQ_TRIS = 1;
}

void
rfm12_Init(void) {
    //RFM12 - Receiver Control Command
    //p16 : 0=INTin, 1=VDIout = 1
    //VDI response time = Med
    //Receiver bandwidth = 134 kHz
    //LNA gain = 0 dB
    //RSSI threshold = -103 dBm
    spi_Command(RFM12_CMD_RXCTL | RFM12_RXCTL_P16 | RFM12_RXCTL_VDI_MED | RFM12_RXCTL_LNA_0 | RFM12_RXCTL_BW_134 | RFM12_RXCTL_RSSI_103);

    //RFM12 - Datafilter Command
    //al : Clock recovery auto lock = 1
    //ml : Clock recovery lock control = 0
    //s : 0=digital, 1=analog filter = 0
    //DQD threshold = 4
    //C2AC
    spi_Command(RFM12_CMD_FILTER | RFM12_FILTER_AL | 0x04);

    //RFM12 - Wake-Up Timer Command
    //T wakeup = 0 ms
    spi_Command(RFM12_CMD_WAKEUP); // disable wakeuptimer

    //RFM12 - Low Duty-Cycle Command
    //en : Enable low duty-cycle = 0
    //Duty-cycle = N/A
    spi_Command(RFM12_CMD_DUTYCYCLE); // disable low duty cycle

    //RFM12 - AFC Command
    //AFC automatic mode = Offset VDI=h
    //st : Strobe edge = 1
    //fi : Fine mode = 0
    //oe : Offset register enable = 1
    //en : Calculate offset = 1
    //Range limit = +15fres to -16fres
    //Max. Deviation = +112.5kHz to -120kHz
    spi_Command(RFM12_CMD_AFC | RFM12_AFC_ST | RFM12_AFC_FI | RFM12_AFC_OE | RFM12_AFC_EN | RFM12_AFC_LIMIT_8 | RFM12_AFC_AUTO_ONCE); // AFC settings: autotuning: -10kHz...+7,5kHz

    //RFM12 - TX Configuration Command
    //mp = 0
    //Delta f (fsk) = 60 kHz
    //Output power = 0 dB
    spi_Command(RFM12_CMD_TXCTL | RFM12_TXCTL_POWER_0 | RFM12_TXCTL_MODULATION_60);

    //RFM12 - LB Det./Clock Div. Command
    //Vlb = 2.2 V
    //Clock output = 10 MHz
    spi_Command(RFM12_CMD_LOWBAT_CLK | RFM12_XTAL_1000 | RFM12_LOWBAT_22);

    rfm12_conf.status = spi_Command(RFM12_CMD_STATUS_READ);

    // Read back the status
    unsigned int ret = 0;
    ret = spi_Command(RFM12_CMD_STATUS_READ);

    asm("nop");
    asm("nop");
}

/*
        Initialiase RFM12 to 433Mhz Band with basic config
 */
void
rfm12_Init_433(void) {
    spi_Init();
    rfm12_Init();

    // Setup 433Mhz Band
    spi_Command(RFM12_CMD_CONFIG | RFM12_BAND_433 | RFM12_CAP_120 | RFM12_CONFIG_EF | RFM12_CONFIG_EL);

    _goto_quiettime_state();
}

/*
        Sets the RF Baud Rate
        must be the same at both ends i.e. Tx and Rx
	
        parameters:
                baud: baudrate in bps  e.g. 19200 or 2400
 */
void
rfm12_SetBaud(unsigned int baud) {
    if (baud < 664)
        baud = 664; // min allowed baudrate
    if (baud < 5400) // 344827.58621 / (1 + cs*7) / (baud-1)
        spi_Command(RFM12_CMD_DATARATE | RFM12_BITRATE_CS | (43104 / baud)); //CS=1 (344828/8)/baud-1
    else
        spi_Command(RFM12_CMD_DATARATE | (344828 / baud)); //CS=0 344828/Baud-1

}

/*
        Sets the Tx Power and Modulation
 */
void
rfm12_SetPower(unsigned char power, unsigned char modulation) {
    spi_Command(RFM12_CMD_TXCTL | power | modulation);
}

/*
        Sets the RF frequency of the RFM12
	
        Parameters
                freq: Centre Frequency bit value
		
                the bit value can be calculaed by:
		
                433MHz(FREQ-430)/0.0025  where FREQ is centre frequency in MHz
                915MHz(FREQ-900)/0.0075  where FREQ is centre frequency in MHz
		
                or use the helper macros to calculate the register bit value
		
                        RFM12_FREQ_433(FREQ)
                        RFM12_FREQ_915(FREQ)
		
                e.g. rfm12_SetFrequency(RFM12_FREQ_915(921.25));
                this sets the frequency to 921.25MHz
 */
void
rfm12_SetFreq(unsigned int freq) {
    if (freq < 0x60) // lower limit 430.2400MHz or 900.72MHZ
        freq = 0x60;
    else if (freq > 0xF3F) // upper limit 439.7575MHz or 929.2725MHz
        freq = 0xF3F;

    spi_Command(RFM12_CMD_FREQ | freq);
}

/*

  Write a Command to the RFM Module using SPI
  
  Requires: 16bit valid command
  Returns:  16bit result from transaction
  
  This is a bi-directional transfer.  
  A command is clocked out to the RFM a bit at a time.  
  At the same time a result is clocked back in a bit at a time.
  
  This code will successfully work to at least 64Mhz.  The 
  clock must be high for at least 25ns (see "Timing Specification" 
  in datasheet).  Even at 64Mhz, a single instruction takes 62.5ns.
   
   Assumes on Entry:
        SEL=1
        RFM12_SCK=0
        RFM12_SDO=any

   On Exit:
        SEL=1
        RFM12_SCK=0
        RFM12_SDO=any
 */
unsigned int
spi_Command(unsigned int cmd) {
    unsigned int ret = 0; // Holds the received RFM12_SDI
    unsigned char n = 16;

    RFM12_CS = 0; // Select the RFM12

    // Send all 16 bits, MSB first
    while (n--) {
        if (cmd & 0x8000) {
            RFM12_SDO = 1; // Write a 1 via SPI
        } else {
            RFM12_SDO = 0; // Write a 0 via SPI
        }

        RFM12_SCK = 1;

        cmd <<= 1;
        ret <<= 1;

        if (RFM12_SDI)
            ret |= 0x0001; // Shift the return value into the device

        RFM12_SCK = 0;
    }

    RFM12_CS = 1;
    /* @todo Verify timings here - why are there 5x NOPs? */
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");

    return ret;
}

/*
        Turns on the Transmitter
 */
void rfm12_EnableTx(void) {
    _goto_transmitting_state();
}

void rfm12_DisableTx(void) {
    _goto_quiettime_state();
}

void rfm12_EnableRx(void) {
    _goto_receiving_state();
}

void rfm12_DisableRx(void) {
    _goto_quiettime_state();
}

/*
 Handles the interrupt generated by the nIRQ of the RFM12 
 
 This routine should be used as a callback from the main application 
 interrupt routine.
 
 If we get here then its for one of the following reasons:
 
         (in bit order as clocked out as status bit)
          /RGIT - TX Register Ready to receive the next byte(Cleared by Transmit Write)
          \FFIT - RX FIFO has reached preprogrammed limit	(Cleared by any FIFO Read)
          POR  - Power-on reset occurred					(cleared on status read)
          /RGUR - TX Register underrun, register overwrite	(cleared on status read)
          \FFOV - RX FIFO Overflow							(cleared on status read)
          WKUP - Wakeup Timer overflow   					(cleared on status read)
          EXT  - LOW on External Pin 16
          LBD  - Low battery detected, powersupply is below the preprogrammed threshold
 */
void rfm12_Handle_Interrupt(void) {
    rfm12_conf.status = spi_Command(RFM12_CMD_STATUS_READ); // Updates the config data with latest Status

    printf("RFM12 INTERRUPT HANDLER, status: %u\r\n", rfm12_conf.status);

    if (rfm12_conf.status & RFM12_STATUS_RGIT_FFIT) // Check if ready to Rx or Tx byte
    {
        printf("INT: In RGIT/FFIT\r\n");
        switch (rfm12_conf.state) // Check Tx or Rx
        {
            case RFM12_STATE_RECEIVING:
                //TODO:							// handle Rx Interrupt
                printf("INT: RGIT\r\n");
                break;

            case RFM12_STATE_TRANSMITTING:
                //TODO:							// handle Tx interrupt
                printf("INT: FFIT\r\n");
                break;
        }
    }

    if (rfm12_conf.status & RFM12_STATUS_POR) // Check Power on reset
    {
        printf("INT: POR\r\n");
        //TODO: Set a flag (perhaps rfm12_conf.state to UNINITIALISED?)
    }

    if (rfm12_conf.status & RFM12_STATUS_RGUR_FFOV) // Check RGUR /Check FFOV status bit
    {
        printf("In RGUR/FFOV\r\n");
        
        switch (rfm12_conf.state) // Check Tx or Rx
        {
            case RFM12_STATE_RECEIVING:
                //TODO:						// handle Rx Interrupt
                printf("INT: RGUR\r\n");
                break;

            case RFM12_STATE_TRANSMITTING:
                //TODO:						// handle Tx interrupt
                printf("INT: FFOV\r\n");
                break;

            // If the RFM12 is uninitialised, or TX/RX finished and the blocks
            // were turned off, we can still get here.
            default:
                break;
        }
    }

    if (rfm12_conf.status & RFM12_STATUS_WKUP) // Check Wakeup status bit
    {
        printf("INT: WKUP\r\n");
        //TODO: handle Wakeup
    }

    if (rfm12_conf.status & RFM12_STATUS_EXT) // Check EXT Interrupt status bit
    {
        printf("INT: EXT\r\n");
        //TODO: handle External
    }

    if (rfm12_conf.status & RFM12_STATUS_LBD) // Check Low battery status bit
    {
        printf("INT: LBD\r\n");
        //TODO: handle Low Battery
    }
}

/* 
  Waits for RFM 12 to become ready by checking first bit of status register
  Before we can Tx or Rx data the RFM12 needs to be ready, 
  i.e. not in the middle of transmitting a previous byte
  
  This function is blocking and will only return when it is ready to Tx or Rx
  
  Assumes:
        RFM12_SCK=0 on entry
        SEL=1
        RFM12_SDO=any
	
  On Exit:
        RFM12_SCK=0
        RFM12_SDO=0
        SEL=1
 */
void rfm12_WaitReady(void) {
#ifdef SIMULATOR						// always return for simulator
    return;
#endif
    char ready = 0;

    while (ready == 0) {
        RFM12_CS = 0; // Select the RFM12
        RFM12_SDO = 0; // write a 0 i.e. start of status command
        RFM12_SCK = 1;

        // Check RGIT bit of status (first bit)
        // If HIGH then Tx/Rx is ready to accept the next byte
        if (RFM12_SDI == 1) 
        { 
            ready = 1;
        }
        
        RFM12_SCK = 0;
        RFM12_CS = 1; // Finished, release RFM12
    }
}

/* 
  Waits for RFM12 to become ready by checking first bit of status register
  Before we can Tx or Rx data the RFM12 needs to be ready, 
  i.e. not in the middle of transmitting or receiving 
  
  This will return after a set amount of time
  
  Returns:  0 = Not ready/Timeout
                        1 = Ready
 */
unsigned char rfm12_isReady(void) {
#ifdef SIMULATOR
    return 1; // always return true for debug
#endif

    char ready = 0;
    unsigned char timeout = 1000;

    // Assumed on entry?
    //LOW_RFM12_SCK();

    

    while ((ready == 0) && (timeout != 0)) {
        RFM12_CS = 0; // !cs LOW - enable
        RFM12_SDO = 0; // clock out a 0 - status command request
        RFM12_SCK = 1; // RFM12_SCK HIGH
        if (RFM12_SDI == 1) // check RGIT bit of status (first bit)
        { // If HIGH then Tx/RX is ready to accept the next byte
            ready = 1;
        }
        timeout--; // check time
        RFM12_SCK = 0; // RFM12_SCK LOW
        RFM12_CS = 1; // !cs HIGH - disable
    }

    if (timeout == 0)
        return 0; // Not Ready - timeout
    else
        return 1; // Ready
}

/*
        loads a byte into the tx buffer
 */
void rfm12_Load_Byte(unsigned char data) {
    rfm12_conf.buffer[rfm12_conf.txrx_pointer] = data;
    rfm12_conf.txrx_pointer++;
}

/*
        Reads and returns data from the FIFO of rfm12
 */
unsigned char rfm12_ReadFifo(void) {
    rfm12_WaitReady(); // wait until byte is received
    unsigned char ret = (unsigned char) spi_Command(RFM12_CMD_FIFOREAD) & 0xFF;
    return ret; // return the contents of fifo
}

/*
        Sets the bandwidth, gain and drssi for the Receiver
 */
void rfm12_SetBandwidth(unsigned char bandwidth, unsigned char gain, unsigned char drssi) {
    spi_Command(RFM12_CMD_RXCTL | bandwidth | gain | drssi); // P16 enabled
}

/*
  Sends the tx buffer

  Blocking - will not return until all bytes sent
  
  Assumes that RF is enabled
  
 */
void rfm12_Tx_Buffer(void) {
    rfm12_conf.txrx_counter = rfm12_conf.txrx_pointer;
    rfm12_conf.txrx_pointer = 0;

    if (rfm12_conf.txrx_counter == 0) // nothing to send so exit
        return;

    while (rfm12_conf.txrx_counter != 0); // tx until the buffer is empty
    {
        rfm12_Tx_Byte(rfm12_conf.buffer[rfm12_conf.txrx_pointer++]);
        rfm12_conf.txrx_counter--;
    }

    rfm12_conf.txrx_pointer = 0; // reset the buffer
}

/*
        Receives RF data into buffer
        parameter:
                count = number of bytes to receive
 */
void rfm12_Rx_Data(unsigned char count) {

    rfm12_conf.txrx_counter = count;
    rfm12_conf.txrx_pointer = 0;

    if (rfm12_conf.txrx_counter == 0) // nothing to receive so exit
        return;

    while (rfm12_conf.txrx_counter != 0); // rx until the we reach the count
    {
        rfm12_conf.buffer[rfm12_conf.txrx_pointer++] = rfm12_ReadFifo(); // get data from RFM12 fifo
        rfm12_conf.txrx_counter--;
    }

    rfm12_conf.txrx_pointer = 0; // reset the buffer
}

/*
        Resets the FIFO on the RFM12 and enabled interrupt at 8 bits
 */
void
rfm12_ResetFifo(void) {
    spi_Command(RFM12_CMD_FIFORESET | RFM12_FIFORESET_DR); // disable FIFO
    spi_Command(RFM12_CMD_FIFORESET | RFM12_FIFORESET_DR | RFM12_FIFORESET_FF | 0x80); // Enable FIFO and ready to receive next Byte
}

// resets the txrx buffer

void
rfm12_Init_Buffer(void) {
    rfm12_conf.txrx_pointer = 0;
    return;
}

/*
  Transmits a single byte via RF Transmitter
 
        Blocking - will not return until byte transmitted

        assumes: el bit (bit 7) is set in config register
        assumes: er bit (bit 8) is cleared pwr mgmt register
        assumes: et bit (bit 5) is set in pwr mgmt register
 */
void
rfm12_Tx_Byte(unsigned char data) {
    rfm12_WaitReady(); // Wait until RFM is ready to Tx
    spi_Command(RFM12_CMD_TX | data);
}

/*******************************************************************************
 * State machine
 ******************************************************************************/

/*
        transmitting state
 */
void
_goto_transmitting_state(void) {
    rfm12_conf.state = RFM12_STATE_TRANSMITTING;

    //RFM12 - Power Management Command
    //er : Enable receiver chain = 0
    //ebb : Enable baseband = 0
    //et : Enable PLL,PA, TX = 1  - auto turn on TX chain
    //es : Enable synthesizer = 0
    //ex : Enable crystal oscillator = 0
    //eb : Enable low battery detector = 0
    //ew : Enable wake-up timer = 0
    //dc : Disable clock output = 0
    spi_Command(RFM12_CMD_PWRMGT | RFM12_PWRMGT_ET);
}

/*
  Receiving state
 */
void
_goto_receiving_state(void) {
    rfm12_conf.state = RFM12_STATE_RECEIVING;

    //RFM12 - Power Management Command
    //er : Enable receiver chain = 1
    //ebb : Enable baseband = 0
    //et : Enable PLL,PA, TX = 0
    //es : Enable synthesizer = 0
    //ex : Enable crystal oscillator = 0
    //eb : Enable low battery detector = 0
    //ew : Enable wake-up timer = 0
    //dc : Disable clock output = 0
    spi_Command(RFM12_CMD_PWRMGT | RFM12_PWRMGT_ER);
}

/*
        quiet time state
 */
void
_goto_quiettime_state(void) {
    rfm12_conf.state = RFM12_STATE_QUIETTIME;
    //RFM12 - Power Management Command
    //er : Enable receiver chain = 0
    //ebb : Enable baseband = 0
    //et : Enable PLL,PA, TX = 0
    //es : Enable synthesizer = 0
    //ex : Enable crystal oscillator = 1
    //eb : Enable low battery detector = 0
    //ew : Enable wake-up timer = 0
    //dc : Disable clock output = 0
    spi_Command(RFM12_CMD_PWRMGT | RFM12_PWRMGT_DC); // DISABLE CRYSTAL OSCILLATOR TO SAVE BATTERY | RFM12_PWRMGT_EX );
    // ALSO DISABLE CLOCK OUTPUT
}

/*
        powering transmitter state
 */
void
_goto_poweringtransmitter_state(void) {
    rfm12_conf.state = RFM12_STATE_POWERINGTRANSMITTER;
    //RFM12 - Power Management Command
    //er : Enable receiver chain = 0
    //ebb : Enable baseband = 0
    //et : Enable PLL,PA, TX = 0
    //es : Enable synthesizer = 0
    //ex : Enable crystal oscillator = 1
    //eb : Enable low battery detector = 0
    //ew : Enable wake-up timer = 0
    //dc : Disable clock output = 0
    spi_Command(RFM12_CMD_PWRMGT | RFM12_PWRMGT_EX);
}
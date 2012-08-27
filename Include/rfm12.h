#ifndef _RFM12_H_
#define _RFM12_H_

/**********************************************************
* rfm12.h
*
* HopeRf RFM12 FSK Wireless Module Library
*
* Adapted from the original rfm12.h by David.
*
* Copyright (c) 2007 Stephen Eaton
* 	seaton@everythingrobotics.com
*
* ===========================================================
* License:
*
*   GPLv3 http://www.gpl.org/ 
*
* ===========================================================
*	History:
*
*	20080122 seaton - optimised WriteCMD
*					- started implementation of spi_write_sw - not working
*	20080103 seaton - commands and options now grouped in rfm12.h
*	20071219 seaton - minor changes to rfm12.h
*	20071205 seaton - Initial ALPHA Release
*
**********************************************************/

/*
  Define the SPI Connection Here  
  
      PIC				 RFM12
   +-------+           +-------+
   |       |		   |       |
   |     CS+-----------+nSEL   |
   |    SDO+-----------+SDI	   |
   |    SDI+-----------+SDO    |
   |    SCK+-----------+SCK    |
   |   nIRQ+-----------+nIRQ   |
   |       |           |       |
   +-------+		   +-------+
   
 The Pins defined below are looking from the PIC side
 
 NOTE:  If not using the CLK as a clock source then startup of RFM12 can be 500mSec
   
*/  

// Required Ports
#define RFM12_IRQ	LATB1
#define RFM12_CS	LATB4
#define RFM12_SCK	LATB3
#define RFM12_SDO	LATB2
#define	RFM12_SDI	PORTBbits.RB5	// INPUT

// TRIS Register
#define RFM12_IRQ_TRIS	TRISB1
#define RFM12_CS_TRIS	TRISB4
#define RFM12_SCK_TRIS	TRISB3
#define RFM12_SDO_TRIS	TRISB2
#define	RFM12_SDI_TRIS	TRISB5

////////////////////////////////////////////////////////////
// RFM12 SPI CMDs 
// POR = Power on Reset value
// 

/*
 RFM12_CMD_CONFIG
 BIT  15 14 13 12 11 10 9 8  7  6  5  4  3  2  1  0
      1   0  0  0  0  0 0 0  EL EF B1 B0 X3 X2 X1 X0 
      
   	B1:B0 - Band of module
	
	The band is module specific and needs to be matched to the physical 
	RFM12 module you have.  If in doubt check the back of the RFM module.
	
	i.e. you can't set 915Mhz if your module is physically a 433Mhz   
    
    X3:X0 - Crystal load Capacitor
 
	Can be used to adjust/Calibrate the XTAL Frequency startup time   
*/
#define RFM12_CMD_CONFIG	0x8000		// Configuration
#define RFM12_CONFIG_EF		0x40		// Enables FIFO Mode
#define RFM12_CONFIG_EL		0x80		// Enable internal data register
#define RFM12_BAND_NONE		0x00		// POR
#define RFM12_BAND_433		0x10		// 433MHz
#define RFM12_BAND_868		0x20		// 868MHz 	// Not implemented
#define RFM12_BAND_915		0x30		// 915MHz

#define RFM12_CAP_085		0x00		//  8.5pF
#define RFM12_CAP_090		0x01		//  9.0pF
#define RFM12_CAP_095		0x02		//  9.5pF
#define RFM12_CAP_100		0x03		// 10.0pF
#define RFM12_CAP_105		0x04		// 10.5pF
#define RFM12_CAP_110		0x05		// 11.0pF
#define RFM12_CAP_115		0x06		// 11.5pF
#define RFM12_CAP_120		0x07		// 12.0pF
#define RFM12_CAP_125		0x08		// 12.5pF - POR
#define RFM12_CAP_130		0x09		// 13.0pF
#define RFM12_CAP_135		0x0A		// 13.5pF
#define RFM12_CAP_140		0x0B		// 14.0pF
#define RFM12_CAP_145		0x0C		// 14.5pF
#define RFM12_CAP_150		0x0D		// 15.0pF
#define RFM12_CAP_155		0x0E		// 15.5pF
#define RFM12_CAP_160		0x0F		// 16.0pF 

/*
 RFM12_CMD_POWERMGMT
 BIT  15 14 13 12 11 10 9 8  7  6   5  4  3  2  1  0
      1   0  0  0  0  0 1 0  ER EBB ET ES EX EB EW DC 
*/
#define RFM12_CMD_PWRMGT		0x8200		// Power management
#define RFM12_PWRMGT_DC			0x01		// Disable Clock (CLK)
#define RFM12_PWRMGT_EW			0x02		// Enable wakeup
#define RFM12_PWRMGT_EB			0x04		// Enable low battery detector
#define RFM12_PWRMGT_EX			0x08		// Enable xtal Oscillator
#define RFM12_PWRMGT_ES			0x10		// enable synthesizer
#define RFM12_PWRMGT_ET			0x20		// enable transmitter
#define RFM12_PWRMGT_EBB		0x40		// enable base band block
#define RFM12_PWRMGT_ER			0x80		// enable receiver

/*
 RFM12_CMD_FREQ
 BIT  15 14 13 12 11  10   9  8  7  6  5  4  3  2  1  0
      1   0  1  0 F11 F10 F9 F8 F7 F6 F5 F4 F3 F2 F1 F0
*/
/* 
	F11:F0  - Frequency Control

	Frequency is an 11bit value 
	calculated by:
	
	f0 = 10 * C1 * (C2 + F/4000) [MHz]
	
	 or
	
	F = (f0 - 900)/0.0075   (915MHz)
	F = (f0 - 430)/0.0025	(433MHz)
	
	Where:
	
	f0	= Centre Frequency in MHz
	F 	= 11 bit register value maps to f11:f0 of the Frequency CMD Register (A000)
	
	Band (MHz)	C1	C2
	====================
	433			1	43
	868			2	43
	915			3	30

*/
#define RFM12_CMD_FREQ		  0xA000							// Frequency control
#define RFM12_FREQ_433(FREQ)  (unsigned int)((FREQ-430)/0.0025)	// Calculate the RFM12 register value for a given Frequency at 433MHz
#define RFM12_FREQ_915(FREQ)  (unsigned int)((FREQ-900)/0.0075)	// Calculate the RFM12 register value for a given Frequency at 915MHz

/*
 RFM12_CMD_DATARATE
 BIT  15 14 13 12 11 10 9 8  7  6   5  4  3  2  1  0
      1   1  0  0  0  1 1 0  CS R6 R5 R4 R3 R2 R1 R0 
      
    R6:R0 - RF bitrate 

	7 bit value
	
	Value of register is calculated as:
	
	bitRate = 344828 / (1 + cs*7) / (BR- 1)
		
	Where:
	bitrate = 7 bit value
	cs = 0 or 1
	BR = Required bitrate in kbps  i.e. 9.600 = 9600bps

	So any value from 600 - 115200 can be calculated
	below is some standard ones and their CS value
*/
#define RFM12_CMD_DATARATE		0xC600		// Data rate
#define RFM12_BITRATE_CS		0x80		// CS bit

// some common preset bitrates 
#define RFM12_BITRATE_115200	0x02	// 115.2 kbps	CS=0
#define RFM12_BITRATE_57600		0x05	// 57600 bps	CS=0
#define RFM12_BITRATE_19200		0x11	// 19200 bps  	CS=0
#define RFM12_BITRATE_9600		0x23	// 9600  bps 	CS=0
#define RFM12_BITRATE_4800		0x47	// 4800 bps  	CS=0
#define RFM12_BITRATE_2400		0x8F	// 2400 bps  	CS=1
#define RFM12_BITRATE_1200		0xA3	// 1200	bps  	CS=1

/*
 RFM12_CMD_RX_CTL
 BIT  15 14 13 12 11 10  9   8  7  6  5  4  3  2  1  0
      1   0  0  1  0 P16 D1 D0 I2 I1 I0 G1 G0 R2 R1 R0
     
   	D1:D0 - VDI ( Valid Data Indicator) response time 
     
    I2:I0 - Bandwidth
 
	Table of optimal bandwidth and transmitter 
	deviation settings for given data rates 
 
	(the data sheet was a bit vague in this area and 
	did not specify frequencies etc so I don't know 
	how valid they are at different frequency bands 
	but they could be used as a starting point)
 
	data rate		bandwidth		Modulation
	1200bps			67kHz			45kHz
	2400			67				45
	4800			67				45
	9600			67				45
	19200			67				45
	38400			134				90
	57600			134				90
	115200			200				120
	
	G1:G0 - Rx Low noise amplifier Relative to Max 
    
    R2:R0 - Relative Signal Strength Indicator
	
	As the RSSI Threshold relies on the LNA gain
	the true RSSI can be calculated by:
	
	RDDIth 	= RSSIsetth + Glna
	
   Where:
	RSSIth 	= RSSI Threshold (Actual)
	RSSIsetth = RSSI Set Threshold (from below)
	Glna	= Gain LNA	 (from below)
    
*/
#define RFM12_CMD_RXCTL			0x9000		// Receiver control
#define RFM12_RXCTL_P16			0x400		// Pin 16 function - VDI output or interrupt input

#define RFM12_RXCTL_VDI_FAST		0x000	// POR
#define RFM12_RXCTL_VDI_MED			0x100	 
#define RFM12_RXCTL_VDI_SLOW		0x200
#define RFM12_RXCTL_VDI_ALWAYS_ON	0x300

#define RFM12_RXCTL_LNA_0		0x00		// -0dB - POR
#define RFM12_RXCTL_LNA_6		0x08		// -6dB 
#define RFM12_RXCTL_LNA_14		0x10		// -14dB 
#define RFM12_RXCTL_LNA_20		0x18		// -20dB 

#define RFM12_RXCTL_BW_400		0x20		// 400kHz
#define RFM12_RXCTL_BW_340		0x40		// 340kHz
#define RFM12_RXCTL_BW_270		0x60		// 270kHz
#define RFM12_RXCTL_BW_200		0x80		// 200kHz - POR 
#define RFM12_RXCTL_BW_134		0xA0		// 134kHz
#define RFM12_RXCTL_BW_67		0xC0		// 67kHz

#define RFM12_RXCTL_RSSI_103	0x00		// -103dB - POR
#define RFM12_RXCTL_RSSI_97		0x01		// -97dB 
#define RFM12_RXCTL_RSSI_91		0x02		// -91dB 
#define RFM12_RXCTL_RSSI_85		0x03		// -85dB 
#define RFM12_RXCTL_RSSI_79		0x04		// -79dB 
#define RFM12_RXCTL_RSSI_73		0x05		// -73dB 


/*
 RFM12_CMD_FILTER
 BIT  15 14 13 12 11 10 9 8  7  6  5 4 3 2  1  0
      1   1  0  0  0  0 1 0  AL ML 1 S 1 F2 F1 F0
      
  F2:F0 DQD threshold parameter. 
  
  Note: To let the DQD report "good signal quality" the threshold parameter should be 4 
  in cases where the bitrate is close to the deviation. At higher deviation/bitrate settings, 
  a higher threshold parameter can report "good signal quality" as well. 
  
 */
#define RFM12_CMD_FILTER		0xC228		// Filter control
#define RFM12_FILTER_S			0x10		// Filter Type 0=digital 1=analogue
#define RFM12_FILTER_ML			0x40		// Clock recovery lock control
#define RFM12_FILTER_AL			0x80		// Clock recovery lock


/*
 RFM12_CMD_FIFORESET_CTL
 BIT  15 14 13 12 11 10 9 8  7  6   5  4 3   2  1  0
      1   1  0  0  1  0 1 0  F3 F2 F1 F0 SP AL FF DR
      
    F3:F0 - FIFO INT level. The FIFO generates INT when the number of received data bits reaches this level.
   
 */
#define RFM12_CMD_FIFORESET		0xCA00		// FIFO and reset control
#define RFM12_FIFORESET_DR		0x01		// Disable Reset Mode
#define RFM12_FIFORESET_FF		0x02		// FIFO fill enabled after reception of Sync pattern
#define RFM12_FIFORESET_AL		0x04		// Fifo Fill start condition 0=sync, 1=Always fill
#define RFM12_FIFORESET_SP		0x08		// Length of Sync Patten 0=2 bytes (0x2DD4), 1=1byte(0xD4)

/*
 RFM12_CMD_SYNCPATTERN
 BIT  15 14 13 12 11 10 9 8  7  6  5   4  3  2  1  0
      1   1  0  0  1  1 1 0  B7 B6 B5 B4 B3 B2 B1 B0
      
      B7:B0 - used for synchron pattern detection can be reprogrammed.
		Default is D4
 */
#define RFM12_CMD_SYNCPATTERN	0xCE00		// Sync pattern

/*
 RFM12_CMD_FIFOREAD
 BIT  15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0
      1   0  1  1  0  0 0 0 0 0 0 0 0 0 0 0
      
      The controller can read 8 bits from the receiver FIFO. Bit 6 (ef) must be set in Configuration Setting Command. 
      
     Note: During FIFO access fSCK cannot be higher than fref /4, where fref is the crystal oscillator frequency. When the duty-cycle of the 
     clock signal is not 50 % the shorter period of the clock pulse width should be at least 2/fref  
 */
#define RFM12_CMD_FIFOREAD		0xB000		// FIFO read command

/*
 RFM12_CMD_AFC_CTL
 BIT  15 14 13 12 11 10 9 8  7  6    5   4  3  2  1  0
      1   1  0  0  0  1 0 0  A1 A0 RL1 RL0 ST FI OE EN
      
  	RFM12_AFC_PWRUP  Recommended for Max distance
	RFM12_AFC_VDI 	 Recommended for receiving from Multiple Transmitters (Point To Multipoint) 
	RFM12_AFC_nVDI	 Recommended for receiving from Single Transmitter (PtP)    
	
	
	RL1:RL1 - Limits the value of the frequency offset register to the next values
	
	fRes:
		315,433Mhz bands: 	2.5kHz
		868MHz band:		5.0kHz
		915MHz band:		7.5kHz
		
	     In automatic operation mode (no strobe signal is needed from the microcontroller to update the output offset register) the AFC circuit 
		 is automatically enabled when the VDI indicates potential incoming signal during the whole measurement cycle and the circuit 
         measures the same result in two subsequent cycles. 

         There are three operation modes, examples from the possible application: 

         1, (a1=0, a0=1) The circuit measures the frequency offset only once after power up. This way, extended TX-RX maximum distance 
         can be achieved. 
         Possible application: 
         In the final application, when the user inserts the battery, the circuit measures and compensates for the frequency offset caused by 
         the crystal tolerances. This method allows for the use of a cheaper quartz in the application and provides protection against tracking 
         an interferer. 

         2a, (a1=1, a0=0) The circuit automatically measures the frequency offset during an initial effective low data rate pattern - easier to 
         receive- (i.e.: 00110011) of the package and changes the receiving frequency accordingly. The further part of the package can be 
         received by the corrected frequency settings. 

         2b, (a1=1, a0=0) The transmitter must transmit the first part of the packet with a step higher deviation and later there is a possibility 
         of reducing it. 
         In both cases (2a and 2b), when the VDI indicates poor receiving conditions (VDI goes low), the output register is automatically 
         cleared. Use these settings when receiving signals from different transmitters transmitting in the same nominal frequencies. 

         3, (a1=1, a0=1) It's the same as 2a and 2b modes, but suggested to use when a receiver operates with only one transmitter. After a 
         complete measuring cycle, the measured value is kept independently of the state of the VDI signal. 

      
 */
#define RFM12_CMD_AFC			0xC400	// AFC CTL command

#define RFM12_AFC_EN			0x01	// AFC Enable - POR
#define	RFM12_AFC_OE			0x02	// AFC Enable frequency offset register - POR
#define	RFM12_AFC_FI			0x04	// AFC Enable High Accurracy (fine) mode - POR
#define RFM12_AFC_ST			0x08	// AFC Strobe Edge When high AFC latest is stored in offset register

#define RFM12_AFC_AUTO_OFF		0x00	// AFC Controlled by MCU
#define RFM12_AFC_AUTO_ONCE		0x40	// Runs Once at powerup
#define RFM12_AFC_AUTO_VDI		0x80	// Keeps Offset when VDI High
#define RFM12_AFC_AUTO_KEEP		0xC0	// Keeps Offset independent of VDI - POR

#define RFM12_AFC_LIMIT_OFF  	0x0	// No Restriction
#define RFM12_AFC_LIMIT_16		0x1		// +15/-16
#define RFM12_AFC_LIMIT_8		0x2		// +7/-8
#define RFM12_AFC_LIMIT_4		0x3		// +3/-4  - POR


/*
 RFM12_CMD_TXCTL
 BIT  15 14 13 12 11 10 9 8  7  6  5  4 3 2  1  0
      1   0  0  1  1  0 0 MP M3 M2 M1 M 0 P2 P1 P0
      
    P2:P0 	The output power is relative to the maximium 
			power available, which depnds on the actual 
			antenna impedenance.

	M3:M0 	Transmit FSK Modulation Control
	MP    	Modulation Phase
	
	modulation (kHz) = f0 + (-1)^sign * (M +1) * (15kHZ)
	
	Where:
		f0 	 = channel centre frequency
		M 	 = 4 bit binary bumber <m3:m0>
		SIGN = (mp) XOR (Data bit)

			
			Pout
				^
				|		^		|		^
				|		|		|		|
				|		|		|		|
				|		|<dfFSK>|<dfFSK>|
				|		|		|		|
				|		|		|		|
				+-------+-------+-------+--------> fout
								f0
						^				^
						|				+------- mp=0 and FSK=1 or mp=1 and FSK=0
						|
						+------------------------mp=0 and FSK=0 or mp=1 and FSK=1
	
*/
#define RFM12_CMD_TXCTL				0x9800		// Tx config control command
#define RFM12_TXCTL_MP				0x100		// Modulation PHASE			

#define RFM12_TXCTL_POWER_0			0x00		//  0dB - POR
#define RFM12_TXCTL_POWER_3			0x01		// -3dB
#define RFM12_TXCTL_POWER_6			0x02		// -6dB
#define RFM12_TXCTL_POWER_9			0x03		// -9dB
#define RFM12_TXCTL_POWER_12		0x04		// -12dB
#define RFM12_TXCTL_POWER_15		0x05		// -15dB
#define RFM12_TXCTL_POWER_18		0x06		// -18dB
#define RFM12_TXCTL_POWER_21		0x07		// -21dB

#define RFM12_TXCTL_MODULATION_15	0x0		// 15 kHz - POR
#define RFM12_TXCTL_MODULATION_30	0x1		// 35 kHz
#define RFM12_TXCTL_MODULATION_45	0x2		// 45 kHz
#define RFM12_TXCTL_MODULATION_60	0x3		// 45 kHz
#define RFM12_TXCTL_MODULATION_75	0x4		// 75 kHz
#define RFM12_TXCTL_MODULATION_90	0x5		// 90 kHz
#define RFM12_TXCTL_MODULATION_105	0x6		// 105 kHz
#define RFM12_TXCTL_MODULATION_120	0x7		// 120 kHz
#define RFM12_TXCTL_MODULATION_135	0x8		// 135 kHz
#define RFM12_TXCTL_MODULATION_150	0x9		// 150 kHz
#define RFM12_TXCTL_MODULATION_165	0xA		// 165 kHz
#define RFM12_TXCTL_MODULATION_180	0xB		// 180 kHz
#define RFM12_TXCTL_MODULATION_195	0xC		// 195 kHz
#define RFM12_TXCTL_MODULATION_210	0xD		// 210 kHz
#define RFM12_TXCTL_MODULATION_225	0xE		// 225 kHz
#define RFM12_TXCTL_MODULATION_240	0xF		// 240 kHz


/*
 RFM12_CMD_PLL
 BIT  15 14 13 12 11 10 9 8  7  6   5    4   3   2  1  0
      1   1  0  0  1  1 0 0  0 OB1 OB0 LPX DDY DDIT 1 BW0
      
      OB1:OB0  Microcontroller output clock buffer rise and fall time control. 

			ob1 ob0 Selected uC CLK frequency 
			0 0 5 or 10 MHz (recommended) 
			0 1 3.3 MHz 
			1 X 2.5 MHz or less 

 
      BW0 Max bit rate [kbps] Phase noise at 1MHz offset [dBc/Hz] 
	        0 86.2		-107 
			1 256 		-102 

      Note: Needed for optimization of the RF 
			performance. Optimal settings can vary 
			according to the external load capacitance. 

*/
#define RFM12_CMD_PLL			0xCC02		// PLL control command
#define RFM12_PLL_BW0			0x01		// PLL BW Control
#define RFM12_PLL_DDIT			0x04		// Disables Dithering in PLL Loop
#define RFM12_PLL_DDY			0x08		// Phase Detector Delay
#define RFM12_PLL_LPX			0x10		// Low Power mode of Crystal

#define RFM12_PLL_OB_10			0x00		// 5-10MHZ - POR
#define RFM12_PLL_OB_3			0x10		// 3.3MHZ
#define RFM12_PLL_OB_2			0x20		// 2.5Mhz or less



/*
 RFM12_CMD_TX_WRITE
 BIT  15 14 13 12 11 10 9 8  7  6  5  4  3  2  1  0
      1   1  0  0  1  0 0 0  T7 T6 T5 T4 T3 T2 T1 T0
      
      T7:T0 - Byte to transmit Bit 7 (el) must be set in Configuration 
			Setting Command 
 */
#define RFM12_CMD_TX		0xB800		// Tx write buffer command

/*
 RFM12_CMD_WAKEUP
 BIT  15 14 13 12 11 10 9  8  7  6  5  4  3  2  1  0
      1   1  1 R4 R3 R2 R1 R0 M7 M6 M5 M4 M3 M2 M1 M0
      
      The wake-up time period can be calculated by (m7 to m0) and (r4 to r0): 
        Twake-up = 1.03 * M * 2R + 0.5 [ms] 
         Note: 
          - For continual operation the ew bit should be cleared and set at the end of every cycle. 
		  - For future compatibility, use R in a range of 0 and 29. 
 */
#define RFM12_CMD_WAKEUP		0xE000		// Wakeup timer command

/*
 RFM12_CMD_DUTYCYCLE
 BIT  15 14 13 12 11 10 9 8  7  6  5  4  3  2  1  0
      1   1  0  0  1  0 0 0 D6 D5 D4 D3 D2 D1 D0 EN
      
     With this command, Low Duty-Cycle operation can be set in order to decrease the average power consumption in receiver mode. 
     
	 The time cycle is determined by the Wake-Up Timer Command. 
	 
     The Duty-Cycle can be calculated by using (d6 to d0) and M. (M is parameter in a Wake-Up Timer Command.) 
     Duty-Cycle= (D * 2 +1) / M *100% 
     
     The on-cycle is automatically extended while DQD indicates good received signal condition (FSK transmission is detected in the 
     frequency range determined by Frequency Setting Command plus and minus the baseband filter bandwidth determined by the 
     Receiver Control Command). 
 */
#define RFM12_CMD_DUTYCYCLE		0xC800		// Duty cycle command
#define RFM12_DUTYCYCLE_EN		0x01		// Enable low duty-cycle mode

/*
 RFM12_CMD_LOWBAT_CLK
 BIT  15 14 13 12 11 10 9 8  7  6  5  4  3  2  1  0
      1   1  0  0  0  0 0 0  D2 D1 D0 0 V3 V2 V1 V0
      
      V3:V0 Low Battery voltage Detector
            
      D2:D0 Divider for the internal RFM12 Oscillator on the 
			External CLK ping can be used as clock source source 
			for a uController.
  
			Also needs the DC bit(bit 0) cleared in the 
			Power management Register to enable CLK Pin.
      
 */

#define RFM12_CMD_LOWBAT_CLK	0xC000		// Low battery detector and CLK pin clock divider

#define RFM12_LOWBAT_22		0x00		// 2.2V  - POR
#define RFM12_LOWBAT_23		0x01		// 2.3V
#define RFM12_LOWBAT_24		0x02		// 2.4V
#define RFM12_LOWBAT_25		0x03		// 2.5V
#define RFM12_LOWBAT_26		0x04		// 2.6V
#define RFM12_LOWBAT_27		0x05		// 2.7V
#define RFM12_LOWBAT_28		0x06		// 2.8V
#define RFM12_LOWBAT_29		0x07		// 2.9V
#define RFM12_LOWBAT_30		0x08		// 3.0V
#define RFM12_LOWBAT_31		0x09		// 3.1V
#define RFM12_LOWBAT_32		0x0A		// 3.2V
#define RFM12_LOWBAT_33		0x0B		// 3.3V
#define RFM12_LOWBAT_34		0x0C		// 3.4V
#define RFM12_LOWBAT_35		0x0D		// 3.5V
#define RFM12_LOWBAT_36		0x0E		// 3.6V
#define RFM12_LOWBAT_37		0x0F		// 3.7V

#define RFM12_XTAL_100		0x00	// 1.00MHz - POR
#define RFM12_XTAL_125		0x20	// 1.25MHz
#define RFM12_XTAL_166		0x40	// 1.66MHZ
#define RFM12_XTAL_200		0x60	// 2.00MHZ
#define RFM12_XTAL_250		0x80	// 2.50MHZ
#define RFM12_XTAL_333		0xA0	// 3.33MHZ
#define RFM12_XTAL_500		0xC0	// 5.00MHZ
#define RFM12_XTAL_1000		0xE0	// 10.00MHZ


/*
	RFM12 Software Reset - undocumented
	
*/
#define RFM12_CMD_RESET		0xFF00	// performs software reset of RFM12 Module

/*
 RFM12_0000_STATUS_READ
 BIT  15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0
      0  X  X  X  X  X  X X X X X X X X X X 
      
      This is result of status read
*/

#define RFM12_CMD_STATUS_READ		0x0000		// Read status Register
#define RFM12_STATUS_RGIT_FFIT		0x8000		// TX ready for next byte or FIFO received data Status
												// depends on mode of transmitter
											
#define RFM12_STATUS_POR			0x4000		// Power on Reset Status
#define RFM12_STATUS_RGUR_FFOV		0x2000		// TX Register underun or RX FIFO Overflow Status
												// depends on mode of transmitter

#define RFM12_STATUS_WKUP			0x1000		// Wakeup Timer overflow Status
#define RFM12_STATUS_EXT			0x0800		// Interrup on external source Status
#define RFM12_STATUS_LBD			0x0400		// Low battery detect Status
#define RFM12_STATUS_FFEM			0x0200		// FIFO Empty Status
#define RFM12_STATUS_ATS			0x0100		// Antenna Tuning Signal Detect Status
#define RFM12_STATUS_RSSI			0x0080		// Received Signal Strength Indicator Status
#define RFM12_STATUS_DQD			0x0040		// Data Quality Dedector Status
#define RFM12_STATUS_CRL			0x0020		// Clock Recovery Locked status
#define RFM12_STATUS_ATGL			0x0010		// Toggling in each AFC Cycle
#define RFM12_STATUS_OFFS_SIGN		0x0008		// Measured Offset Frequency Sign Value 1='+', 0='-' 
#define RFM12_STATUS_OFFS			0x0004		// Measured offset Frequency value (3 bits) 
#define RFM12_STATUS_OFFS_MASK		0x0003		// Measured offset mask


#define isRGITFFIT (1 << RFM12_STATUS_RGIT_FFIT)
/*
#define isPOR  (1 << RFM12_0000_POR)
#define isRGURFFOV (1 << RFM12_0000_RGUR_FFOV)
#define isWKUP (1 << RFM12_0000_WKUP)
#define isEXT (1 << RFM12_0000_EXT)
#define isLBD (1 << RFM12_0000_LBD)
#define isFFEM (1 << RFM12_0000_FFEM)
#define isATS (1 << RFM12_0000_ATS)
#define isRSSI (1 << RFM12_0000_RSSI)
#define isDQD (1 << RFM12_0000_DQD)
#define isCRL (1 << RFM12_0000_CRL)
#define isATGL (1 << RFM12_0000_ATGL)
*/

/*
	.txrx_mode
*/
#define RFM12_MODE_MASTER_RX		0x0		// RX Mode - Master
#define RFM12_MODE_MASTER_TX		0x1		// TX Mode - Master
#define RFM12_MODE_SLAVE_RX			0x2		// RX Mode - Slave
#define RFM12_MODE_SLAVE_TX			0x3		// TX Mode - Slave
//#define RFM12_MODE_RX_HOP	0x2		// RX Mode Freq Hopping
//#define RFM12_MODE_TX_HOP	0x3		// TX Mode Freq hopping

typedef enum 
{ 
   RFM12_STATE_POWERINGTRANSMITTER = 1,
   RFM12_STATE_TRANSMITTING = 2,
   RFM12_STATE_RECEIVING    = 3,
   RFM12_STATE_QUIETTIME    = 4
} RFM12_STATE;

/*---------------------------------------------------------
 NOT IMPLEMENTED
 Most of the configurable bits of the RFM12 chip are mapped
 into this structure
 ---------------------------------------------------------*/
struct _rfm12_t {
	// State
	RFM12_STATE 	state;			// current state of tranceiver
	unsigned char 	channel;			// current channel
	
	unsigned char 	buffer[32];			// tx/rx buffer
	unsigned char 	txrx_pointer;
	unsigned char 	txrx_counter;
	
	unsigned int 	status;				// Readonly - Status register of RFM12
};
typedef struct _rfm12_t rfm12_t;

////////////////////////////////////////////////////////////////////////////////////////////
//
// function definitions
//
// TODO -> Initialise function for 8xx MHz
void rfm12_Init_433(void);				// Initialiases the RFM12 for 433MHz band
void rfm12_Init_915(void);				// Initialiases the RFM12 for 915MHz band

void rfm12_SetBaud(unsigned int baud);			// sets RF baud rate
void rfm12_SetBandwidth(unsigned char bandwidth, unsigned char gain, unsigned char drssi);   // Sets the bandwidth, gain and drssi		
void rfm12_SetFreq(unsigned int freq);			// Sets the Frequency 
void rfm12_SetPower(unsigned char power, unsigned char modulation); // Sets power and modulation of Tx
void rfm12_EnableTx(void);				// Enables Transmitter circuit
void rfm12_EnableRx(void);				// Enables Receiver circuit
void rfm12_DisableTx(void);				// Disables Transmitter
void rfm12_DisableRx(void);				// Disables Receiver

unsigned char rfm12_isReady(void);		// Waits until rfm12 is ready (Tx/Rx) with timeout
void rfm12_WaitReady(void);				// Waits until rfm12 is ready (Tx/Rx) without timeout

void rfm12_Init_Buffer(void);			// Initialiase the Tx/Rx Buffer
void rfm12_Load_Byte(unsigned char data);		// Load a byte into Tx Buffer
unsigned char rfm12_Read_Buffer(unsigned char index);	// reads byte from buffer
void rfm12_Tx_Byte(unsigned char data);			// Transmit a single byte
void rfm12_Tx_Buffer(void);				// Transmit the entire buffer
void rfm12_Rx_Data(unsigned char count);		// Receives data
void rfm12_ResetFifo(void);				// Resets the Rx FIFO buffer on the RFM12
unsigned char rfm12_ReadFifo(void);			// Reads the Fifo and returns data

void _goto_transmitting_state(void);
void _goto_quiettime_state(void);
void _goto_receiving_state(void);
void _goto_quiettime_state(void);

#define rfm12_DisableFifo() spi_Command(RFM12_CMD_FIFORESET | RFM12_FIFORESET_DR)
#define rfm12_EnableFifo() spi_Command(RFM12_CMD_FIFORESET | RFM12_FIFORESET_DR | RFM12_FIFORESET_FF | 0x80 )

// these functions not yet implemented			
void rfm12_Handle_Interrupt(void);		// Interrupt callback						

// Built in, bit banged SPI
void spi_Init(void);
unsigned int spi_Command(unsigned int command);
void spi_Write1(void);
void spi_Write0(void);

void rfm12_Init(void);

/* TODO -> Move to rfm12.c after debugging finished */
// Global variable to this function, contains all the settings
volatile rfm12_t rfm12_conf;

#endif

#ifndef _PACKET_H_
#define _PACKET_H_

// The packet size
#define PACKET_LENGTH 32

// Total header length, confusingly including the CRC trailer
#define HEADER_LENGTH 8

// Total space for data is PACKET_LENGTH minus the size of headers
// and trailer
#define PACKET_DATA_MAX (PACKET_LENGTH - HEADER_LENGTH)

// A unique value that must appear at the start of each packet.
// Enables multiple different "networks" on the same frequency and
// also stops us picking up nonsense from other transmitters.
// One nibble could be used as a version.
#define PACKET_MAGIC 0xAA

// Our node's ID
#define NODE_ADDRESS 0x01

// Uncomment this if you want all stations to receive packets destined for 
// the specified multicast address.
#define PACKET_MULTICAST
#define MULTICAST_ADDRESS 0xFF

// Shared between the packet code and the main application
// The buffer for received data (could also be used for sending data, if not
// worried about receiving whilst building packets).
unsigned char data_buffer[PACKET_DATA_MAX];
// The flag to indicate new data (cleared by application)
bit data_arrived = 0;

/* POSSIBLE FEATURE LIST
 * 
 * - Acknowledgement & retransmissions
 * - Multiple packet buffering (ring buffer?)
 * - Ring buffer shared with the application for TX/RX
 */

/* Packet format is:
 *
 *      magic - 1 byte
 *  from_addr - 1 byte
 *    to_addr - 1 byte
 *     length - 1 byte
 *   sequence - 1 byte
 * (reserved) - 2 bytes (future features)
 *       data - equals PACKET_LENGTH - 8
 *        ...
 *   checksum - 1 byte
 */
/* 
struct packet_header {
	unsigned char magic;
	unsigned char from_addr;
	unsigned char to_addr;
	unsigned char length;
	unsigned char sequence;
	unsigned short reserved;	// Currently unimplemented, but designed
								// to be used for acknowledgement
}; 
*/

unsigned char send_packet(unsigned char, unsigned char*, unsigned char);
unsigned char handle_interrupt();

// Internal functions
unsigned char _process_packet(unsigned char);
unsigned char _receive_packet();

#endif
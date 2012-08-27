#include "rfm12.h"
#include "packet.h"
#include <stdio.h>
#include "delay.h"
#include "crc8.h"

/*
 * Basics for a packet driver
 */

// Used to receive packets (including headers)
unsigned char packet_buffer[PACKET_LENGTH];
unsigned char sequence_number = 0;
unsigned int receive_count = 0;

/*
 * Returns: 0 on failure
 *          1 on success
 */
unsigned char send_packet(unsigned char to_addr, unsigned char *data, unsigned char data_len) {
    // TODO: Wait for timeout to tick down (in multi-station mode)

    // Too much data
    if (data_len > PACKET_DATA_MAX) {
        return 0;
    }

    unsigned char crc;
    crc8_init(0xFF);

    rfm12_EnableTx(); // TODO: turn on transmitter

    rfm12_Tx_Byte(0xAA); // Preamble
    rfm12_Tx_Byte(0xAA);
    rfm12_Tx_Byte(0xAA);

    rfm12_Tx_Byte(0x2D); // RFM12 sync pattern
    rfm12_Tx_Byte(0xD4);

    rfm12_Tx_Byte(PACKET_MAGIC); // Sync bytes
    crc = crc8_byte(PACKET_MAGIC);

    rfm12_Tx_Byte(NODE_ADDRESS); // From
    crc = crc8_byte(NODE_ADDRESS);

    rfm12_Tx_Byte(to_addr); // To
    crc = crc8_byte(to_addr);

    rfm12_Tx_Byte(HEADER_LENGTH + data_len); // Packet length
    crc = crc8_byte(HEADER_LENGTH + data_len);

    rfm12_Tx_Byte(sequence_number); // Sequence
    crc = crc8_byte(sequence_number);
    sequence_number++;

    rfm12_Tx_Byte(0); // Reserved
    crc = crc8_byte(0);

    rfm12_Tx_Byte(0); // Reserved
    crc = crc8_byte(0);

    for (int n = 0; n < data_len; n++) {
        rfm12_Tx_Byte(data[n]);
        crc = crc8_byte(data[n]);
    }

    rfm12_Tx_Byte(crc); // CRC

    rfm12_Tx_Byte(0xAA); // Trailer
    rfm12_Tx_Byte(0xAA);

    rfm12_DisableTx(); // Turn off Tx

    return 1;
}

unsigned char
handle_interrupt() {
    unsigned char bytes_received = _receive_packet();

    rfm12_DisableRx();
    rfm12_DisableFifo();

    // Process the packet, but only if the application has dealt with the
    // last packet.  If not, it will be silently dropped..
    if (!data_arrived) {
        _process_packet(bytes_received);
    }

    rfm12_EnableFifo();
    rfm12_EnableRx();

    return 1;
}

unsigned char
_receive_packet() {
    // TODO:
    // Collision detection, when we RX a packet (even if not for us) then ignore it
    // and wait for a random period before trying to TX (automated backoff)

    unsigned char i = 0;
    // At minimum we need to read an entire header
    unsigned char bytes_to_read = HEADER_LENGTH;

    // Read at least the header length.  Checking the RFM12_IRQ pin does
    // not seem to work here, we only ever get 1 byte at a time.
    while (i < bytes_to_read) {
        packet_buffer[i] = rfm12_ReadFifo(); // read in next Rx byte

        // The third byte should be the length header
        if (i == 3) {
            bytes_to_read = packet_buffer[3];
            if (bytes_to_read > PACKET_LENGTH) {
                return 0;
            }
        }
        i++;
    }

    return i;
}

unsigned char
_process_packet(unsigned char bytes_received) {
    unsigned char length = 0;
    unsigned char crc = 0;
    receive_count++;

    // Check for the start-of-packet byte
    if (packet_buffer[0] != PACKET_MAGIC) {
        return 0;
    }

    // Check that the packet is for us (or, in multicast mode, is for
    // the multicast address)
#ifdef PACKET_MULTICAST
    if (packet_buffer[2] != NODE_ADDRESS && packet_buffer[2] != MULTICAST_ADDRESS) {
#else
    if (packet_buffer[2] != NODE_ADDRESS) {
#endif
        return 0;
    }

    // Extract the length
    length = packet_buffer[3];

    // Calculate the CRC for the whole packet, minus the CRC
    // which is at the end
    crc8_init(0xff);
    for (int n = 0; n < length - 1; n++) {
        crc = crc8_byte(packet_buffer[n]);
    }

    // Check the CRC against the packet
    if (crc != packet_buffer[length - 1]) {
        return 0;
    }

    // Copy the data to the buffer
    for (int n = 0; n < length - HEADER_LENGTH; n++) {
        data_buffer[n] = packet_buffer[7 + n];
    }
    data_arrived = length - HEADER_LENGTH;

    return 1;
}
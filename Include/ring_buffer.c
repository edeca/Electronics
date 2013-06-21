#include "ring_buffer.h"

void ring_buffer_init(ring_buffer *buf, char *data, unsigned char length) {
    buf->data = data;
    buf->length = length;
    buf->read_pos = 0;
    buf->write_pos = 0;
    buf->bytes_lost = 0;
    while (length)
        buf->data[--length] = 0;
}

void ring_buffer_write_byte(ring_buffer *buf, char byte) {
    buf->data[buf->write_pos++] = byte;

    // My limited tests show this is more efficient on an
    // 8 bit PIC than using modulo.
    if (buf->write_pos == buf->length)
        buf->write_pos = 0;

    // We always leave a free element
    if (buf->write_pos == buf->read_pos) {
        buf->bytes_lost++;
        buf->read_pos++;
        if (buf->read_pos == buf->length)
            buf->read_pos = 0;
    }
}

// User needs to check !empty first
char ring_buffer_read_byte(ring_buffer *buf) {
    char ret = buf->data[buf->read_pos];
    buf->read_pos++;
    if (buf->read_pos == buf->length)
        buf->read_pos = 0;
    return ret;
}

char ring_buffer_peek_byte(ring_buffer *buf) {
    return buf->data[buf->read_pos];
}

unsigned char ring_buffer_is_empty(ring_buffer *buf) {
    return buf->read_pos == buf->write_pos;
}

unsigned char ring_buffer_bytes_waiting(ring_buffer *buf) {
    if (buf->write_pos > buf->read_pos)
        return buf->write_pos - buf->read_pos;
    else
        return buf->length - (buf->read_pos - buf->write_pos);
}
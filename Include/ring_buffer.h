typedef volatile struct {
   char *data;
   unsigned char length;
   unsigned char read_pos;
   unsigned char write_pos;
   unsigned char bytes_lost;
} ring_buffer;

void ring_buffer_init(ring_buffer*, char*, unsigned char);
void ring_buffer_write_byte(ring_buffer*, char);
unsigned char ring_buffer_is_empty(ring_buffer *buf);
char ring_buffer_peek_byte(ring_buffer *buf);
char ring_buffer_read_byte(ring_buffer *buf);
unsigned char ring_buffer_bytes_waiting(ring_buffer *buf);
#ifndef _GRAPHICS_H_
#define _GRAPHICS_H_

typedef struct {
	unsigned char x1;
	unsigned char y1; 
	unsigned char x2; 
	unsigned char y2;
} bounding_box_t;

bounding_box_t glcd_string_new(char *string, unsigned char x, unsigned char y, unsigned char *font);
//unsigned char glcd_char_adv(char c, unsigned char x, unsigned char y);
bounding_box_t glcd_char_new(unsigned char c, unsigned char x, unsigned char y, unsigned char *font);
void glcd_char(char c, unsigned char x, unsigned char page);
void draw_rectangle(int x1, int y1, int x2, int y2, char colour);
void draw_box(int x1, int y1, int x2, int y2, char colour);
//unsigned char text_width(char *string);
unsigned char text_width(unsigned char *string, unsigned char *font);
unsigned char text_height(unsigned char *string, unsigned char *font);

#endif // _GRAPHICS_H_

#ifndef LCD_HELPER__H
#define LCD_HELPER__H

#include "../include/st7735s.h"

#define LCD_WIDTH       128
#define LCD_HEIGHT      160
#define CHAR_WIDTH      5
#define CHAR_HEIGHT     7
#define CHAR_H_SPACING  1
#define LINE_HEIGHT     10

#define RED   (color_t){255, 0, 0}
#define GREEN (color_t){0, 255, 0}
#define BLUE  (color_t){0, 0, 255}
#define BLACK (color_t){0, 0, 0}
#define WHITE (color_t){255, 255, 255}
#define CYAN  (color_t){0, 255, 255}
#define YELLOW (color_t){255, 255, 0}
#define MAGENTA (color_t){255, 0, 255}
#define ORANGE (color_t){255, 165, 0}
#define PURPLE (color_t){128, 0, 128}
#define PINK (color_t){255, 192, 203}
#define BROWN (color_t){165, 42, 42}
#define GRAY (color_t){128, 128, 128}

typedef struct {
    unsigned char red;
    unsigned char green;
    unsigned char blue;
} color_t;


lcd_ptr_t lcd_init();
lcd_status_t set_background(color_t bg_color);
lcd_status_t draw_text(short int x, short int y, const char *text, color_t txt_color, int wrap);
lcd_status_t draw_rectangle(unsigned short x0, unsigned short y0, unsigned short x1, unsigned short y1, color_t color);
void lcd_cleanup(lcd_ptr_t my_lcd_settings);

#endif // LCD_HELPER__H
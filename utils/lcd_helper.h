#ifndef lcd_helper__h
#define lcd_helper__h

#include "st7735s.h"

typedef struct {
    unsigned char red;
    unsigned char green;
    unsigned char blue;
} color_t;


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

lcd_status_t draw_text(short int x, short int y, const char *text, color_t txt_color);
void lcd_cleanup(lcd_ptr_t my_lcd_settings);
void animate_text(short int y, const char *text, color_t txt_color, color_t bg_color, int delay_time);
#endif
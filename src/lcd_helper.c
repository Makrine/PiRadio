#include "../include/lcd_helper.h"
#include "../include/font.h"
#include "../include/st7735s.h"
#include <wiringPi.h>
#include <string.h>
#include <stdio.h>

extern int wiringPiInitialized;

lcd_ptr_t lcd_init() {
    if (wiringPiInitialized != 0) {
        printf("LCD: wiring pi not initialized. Initializing...\n");
        if (wiringPiSetup() == -1) {
            printf("LCD: wiring pi setup failed\n");
            return NULL; // Failed to initialize WiringPi
        }
        wiringPiInitialized = 0;
    }
    lcd_ptr_t lcd_settings = setup();

    return lcd_settings;
}

lcd_status_t set_background(color_t bg_color) {
    return set_background_color(bg_color.red, bg_color.green, bg_color.blue);
}

lcd_status_t draw_rectangle(unsigned short x0, unsigned short y0, unsigned short x1, unsigned short y1, color_t color) {
    return lcd_drawFilledRectangle(x0, y0, x1, y1, color.red, color.green, color.blue);
}


lcd_status_t draw_text(short int x, short int y, const char *text, color_t txt_color, int wrap) {
    int size = strlen(text);
    int k = 0;

    for (int i = 0; i < size; i++) {
        if(wrap && (x + k + CHAR_WIDTH > LCD_WIDTH)) {
            y += LINE_HEIGHT;
            k = 0;
        }
        const unsigned char *font5x7 = fonts5x7[text[i] - 32];

        for (int row = 0; row < CHAR_HEIGHT; row++) {
            for (int col = 0; col < CHAR_WIDTH; col++) {
                int px = x + k + col;
                int py = y + row;
                if(px >= 0 && px < LCD_WIDTH && py >= 0 && py < LCD_HEIGHT) {
                    if(font5x7[col] & (1 << row)) {
                        lcd_drawPixel(px, py, txt_color.red, txt_color.green, txt_color.blue);
                    }
                }
            }
        }

        k += CHAR_WIDTH + CHAR_H_SPACING; // 5px char + 1px spacing
    }

    return LCD_OK;
}

void lcd_cleanup(lcd_ptr_t my_lcd_settings) {
    digitalWrite(PIN_CHIP_SELECT, HIGH);
    lcd_deleteSettings(my_lcd_settings);
}
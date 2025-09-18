#include <stdio.h>
#include "st7735s.h"
#include <stdlib.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <stdint.h>
#include <string.h>
#include "font.h"
#include "lcd_helper.h"
#include <pthread.h>
#include <unistd.h> 

#define LCD_WIDTH       128
#define LCD_HEIGHT      160


void animate_text(short int y, const char *text, color_t txt_color, color_t bg_color, int delay_time) {
    for (int i = 128; i > -LCD_WIDTH; i--) {
        lcd_drawFilledRectangle(0, y, LCD_WIDTH - 1, FONT_HEIGHT, bg_color.red, bg_color.green, bg_color.blue); // Clear screen with black background
        draw_text(i, y, text, txt_color);
        delay(delay_time);
    }
    
}

lcd_status_t draw_text(short int x, short int y, const char *text, color_t txt_color) {
    int size = strlen(text);
    int k = 0;
    

    for (int i = 0; i < size; i++) {

        if (k > (LCD_WIDTH - x - 6)) break; // Stop if we exceed screen width
        const unsigned char *font5x7 = fonts5x7[text[i] - 32];

        for (int row = 0; row < 7; row++) {
            for (int col = 0; col < 5; col++) {
                int px = (x + k + col + LCD_WIDTH) % LCD_WIDTH;
                int py = y + row;

                if(py < 0 || py >= LCD_HEIGHT)
                    continue;

                if(font5x7[col] & (1 << row)) {
                    lcd_drawPixel(px, py, txt_color.red, txt_color.green, txt_color.blue);
                }
            }
        }

        k += 6;
    }

    return LCD_OK;
}





void lcd_cleanup(lcd_ptr_t my_lcd_settings) {
    digitalWrite(PIN_CHIP_SELECT, HIGH);
    lcd_deleteSettings(my_lcd_settings);
}


// int main(void) {
    
//     lcd_ptr_t my_lcd_settings = setup();

//     if(set_background_color(0, 255, 255) < LCD_OK) {
//         fail_handler();
//     }
//     // if(draw_text(10, 10, "hello niggaz 123 ! Glory TO Mackiland YOO", RED)    < LCD_OK) {
//     //     fail_handler();
//     // }

//     animate_text();
//     lcd_cleanup(my_lcd_settings);

//     return 0;
// }



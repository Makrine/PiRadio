#include "../include/lcd_helper.h"
#include "../include/st7735s.h"
#include <stdio.h>


int wiringPiInitialized = -1;

int main() {
    lcd_ptr_t lcd_settings = lcd_init();
    if(lcd_settings == NULL)
    {   
        printf("Initialization failed\n");
        return -1;
    }

    set_background(BLACK);

    draw_text(10, 10, "Hello Mackiland! Hello Mackiland! Hello Mackiland! Hello Mackiland!", RED, 1);

    return 0;
}

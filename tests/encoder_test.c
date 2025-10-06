#include "../include/encoder_helper.h"
#include <stdio.h>

int wiringPiInitialized = -1;

int main() {
    if(encoder_init() != 0)
    {   
        printf("Initialization failed\n");
        return -1;
    }

    while(1) {
        encoder_value_t val = read_encoder();
        encoder_delay(5);
        if(val.rotation == 1) printf("Clockwise\n");
        else if (val.rotation == -1) printf("Counterclockwise\n");
        
        if(val.btn) printf("Button\n");
    }

    return 0;
}

#include "encoder_helper.h"
#include <wiringPi.h>

int setup_encoder() {
    // if (wiringPiSetupGpio() != -1) {
    //     return -1; // Failed to initialize WiringPi
    // }
    
    pinMode(PIN_CLK, INPUT);
    pinMode(PIN_DT, INPUT);
    pinMode(PIN_SW, INPUT);
    pullUpDnControl(PIN_CLK, PUD_UP);
    pullUpDnControl(PIN_DT, PUD_UP);
    pullUpDnControl(PIN_SW, PUD_UP);
    return 1;
}

void encoder_delay(int ms) {
    delay(ms);
}

int read_encoder(int *btn) {
    static int last_clk = HIGH;
    int clk = digitalRead(PIN_CLK);
    int dt = digitalRead(PIN_DT);
    int sw = digitalRead(PIN_SW);
    *btn = (sw == LOW) ? 1 : 0; // Button pressed state
    int value = 0;

    if (clk != last_clk) {
        if (clk == LOW) {
            if (dt == LOW) {
                value = -1; // Counter-clockwise
            } else {
                value = 1;  // Clockwise
            }
        }
    }
    last_clk = clk;
    return value;
}

void cleanup_encoder() {
    pinMode(PIN_CLK, INPUT);
    pinMode(PIN_DT, INPUT);
    pinMode(PIN_SW, INPUT);
    pullUpDnControl(PIN_CLK, PUD_OFF);
    pullUpDnControl(PIN_DT, PUD_OFF);
    pullUpDnControl(PIN_SW, PUD_OFF);
}

// int main() {
//     setup_encoder();
//     int btn = 0;
//     while (1) {
//         int change = read_encoder(&btn);
//         if (change != 0) {
//             if (change == 1) {
//                 printf("Rotated Clockwise\n");
//             } else if (change == -1) {
//                 printf("Rotated Counter-Clockwise\n");
//             }
//         }
//         if(btn) {
//             printf("Button Pressed\n");
//         }
//         delay(100);
//     }
//     cleanup_encoder();
//     return 0;
// }
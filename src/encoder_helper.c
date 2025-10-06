#include "../include/encoder_helper.h"
#include <wiringPi.h>
#include <stdio.h>

#define DEBOUNCE_MS 5

static unsigned long last_btn_time = 0;
extern int wiringPiInitialized;

int encoder_init() {
    if (wiringPiInitialized != 0) {
        printf("ENCODER: wiring pi not initialized. Initializing...\n");
        if (wiringPiSetup() == -1) {
            printf("ENCODER: wiring pi setup failed\n");
            return 1; // Failed to initialize WiringPi
        }
        wiringPiInitialized = 0;
    }

    pinMode(PIN_CLK, INPUT);
    pinMode(PIN_DT, INPUT);
    pinMode(PIN_SW, INPUT);
    pullUpDnControl(PIN_CLK, PUD_UP);
    pullUpDnControl(PIN_DT, PUD_UP);
    pullUpDnControl(PIN_SW, PUD_UP);

    return 0;
}

void encoder_delay(int ms) {
    delay(ms);
}

// encoder_value_t read_val() {
//     encoder_value_t encoder_value = {0, 0};
//     static int last_clk = HIGH;
//     static int last_sw = HIGH;

//     int clk = digitalRead(PIN_CLK);
//     int dt = digitalRead(PIN_DT);
//     int sw = digitalRead(PIN_SW);

//     if(sw != last_sw) {
//         encoder_value.btn = !sw;
//         last_sw = sw;
//     }

//     if (clk != last_clk) {
//         if (clk == LOW) {
//             if (dt == LOW) {
//                 encoder_value.rotation = -1; // Counter-clockwise
//             } else {
//                 encoder_value.rotation = 1;  // Clockwise
//             }
//         }
//     }
//     last_clk = clk;

//     return encoder_value;
// }

encoder_value_t read_encoder() {
    encoder_value_t event = {0};

    static int last_state = 0;
    static int accum = 0; // counts steps between detents
    static int last_sw = HIGH; // previous button state

    unsigned long now = millis();
    int clk = digitalRead(PIN_CLK);
    int dt  = digitalRead(PIN_DT);
    int sw  = digitalRead(PIN_SW);
    //printf("clk: %d | dt: %d | sw: %d\n", clk, dt, sw);
    // Button handling with debounce
    if (last_sw == HIGH && sw == LOW) { // detect press (falling edge)
        if (now - last_btn_time > DEBOUNCE_MS) {
            event.btn = 1;          // register button press once
            last_btn_time = now;       // reset debounce timer
        }
    }

    last_sw = sw;

    // Encoder state machine
    int new_state   = (clk << 1) | dt; // two-bit state: 00,01,11,10
    int transition  = (last_state << 2) | new_state;

    int step = 0;
    switch (transition) {
        case 0b0001: case 0b0111: case 0b1110: case 0b1000:
            step = -1; // CCW micro-step
            break;
        case 0b0010: case 0b0100: case 0b1101: case 0b1011:
            step = 1;  // CW micro-step
            break;
        default:
            step = 0;  // Invalid / bounce
            break;
    }

    accum += step;

    // Only report when we reach a stable "detent" position
    if (new_state == 0b00) { 
        if (accum > 0) {
            event.rotation = 1;
        } else if (accum < 0) {
            event.rotation = -1;
        }
        accum = 0; // reset after reporting
    }

    last_state = new_state;
    return event;
}
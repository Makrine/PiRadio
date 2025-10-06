#ifndef ENCODER_HELPER_H
#define ENCODER_HELPER_H

#define PIN_CLK	22 // 6
#define PIN_DT  21 // 5	
#define PIN_SW  25 // 26


typedef struct encoder_value_t {
	int btn; // 1, 0
	int rotation; // -1, 0, 1
} encoder_value_t;

int encoder_init();
encoder_value_t read_encoder();
void encoder_delay(int ms);

#endif // ENCODER_HELPER_H
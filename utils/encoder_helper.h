#ifndef encoder_helper__h
#define encoder_helper__h

#define PIN_CLK 7
#define PIN_DT 9
#define PIN_SW 8

int setup_encoder();
int read_encoder(int *btn);
void cleanup_encoder();
void encoder_delay(int ms);
#endif
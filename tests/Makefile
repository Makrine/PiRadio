CC = gcc
CFLAGS = -Iutils -Wall -Wextra
LIBS = -lwiringPi -lm

TARGET = test
SRC_ENCODER = ../src/encoder_helper.c encoder_test.c
SRC_LCD = ../src/lcd_helper.c ../src/st7735s.c ../src/font.c lcd_test.c 
SRC_RADIO = ../src/radio_helper.c ../src/cJSON.c radio_test.c 

all: $(TARGET)

$(TARGET): $(SRC_ENCODER)
	$(CC) $(CFLAGS) $(SRC_ENCODER) -o $(TARGET) $(LIBS)

debug: CFLAGS += -g -O0
debug: $(TARGET)

lcd:
	$(CC) $(CFLAGS) $(SRC_LCD) -o $(TARGET) $(LIBS)

radio:
	$(CC) $(CFLAGS) $(SRC_RADIO) -o $(TARGET) $(LIBS)

clean:
	rm -f $(TARGET)

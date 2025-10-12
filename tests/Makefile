CC = gcc
CFLAGS = -Iinclude -Wall -Wextra
LIBS = -lwiringPi -lm -lpaho-mqtt3c

TARGET = test
SRC_ENCODER = ../src/encoder_helper.c encoder_test.c
SRC_LCD = ../src/lcd_helper.c ../src/st7735s.c ../src/font.c lcd_test.c 
SRC_RADIO = ../src/radio_helper.c ../src/cJSON.c radio_test.c 
SRC_MQTT = ../src/mqtt_client_helper.c mqtt_test.c ../src/encoder_helper.c ../src/lcd_helper.c ../src/st7735s.c ../src/font.c  ../src/radio_helper.c ../src/cJSON.c


all: $(TARGET)

$(TARGET): $(SRC_ENCODER)
	$(CC) $(CFLAGS) $(SRC_ENCODER) -o $(TARGET) $(LIBS)

debug: CFLAGS += -g -O0
debug: $(TARGET)

lcd:
	$(CC) $(CFLAGS) $(SRC_LCD) -o $(TARGET) $(LIBS)

radio:
	$(CC) $(CFLAGS) $(SRC_RADIO) -o $(TARGET) $(LIBS)

mqtt:
	$(CC) $(CFLAGS) $(SRC_MQTT) -o $(TARGET) $(LIBS)

clean:
	rm -f $(TARGET)

CC = gcc
CFLAGS = -Iutils -Wall -Wextra
SRC = main.c utils/radio_helper.c utils/cJSON.c utils/lcd_helper.c utils/st7735s.c utils/font.c utils/encoder_helper.c
TARGET = main
LIBS = -lwiringPi -lm

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LIBS)

debug: CFLAGS += -g -O0
debug: $(TARGET)

clean:
	rm -f $(TARGET)
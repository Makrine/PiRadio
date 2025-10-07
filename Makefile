CC = gcc
CFLAGS = -Iinclude -Wall -Wextra
LIBS = -lwiringPi -lm
TARGET = main
SRC = src/encoder_helper.c src/radio_helper.c src/cJSON.c src/lcd_helper.c src/st7735s.c src/font.c src/main.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LIBS)

debug: CFLAGS += -g -O0
debug: $(TARGET)

clean:
	rm -f $(TARGET)

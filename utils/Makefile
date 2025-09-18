CC = gcc
CFLAGS = -Wall -Wextra
SRC = lcd_helper.c
TARGET = main
LIBS = -lwiringPi -lm

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LIBS)

debug: CFLAGS += -g -O0
debug: $(TARGET)

clean:
	rm -f $(TARGET)
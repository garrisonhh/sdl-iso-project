CC = gcc
OBJS = src/*.c src/*/*.c
CFLAGS_DEBUG = -Wall -g
CFLAGS_BUILD = -O3
CFLAGS = -ffast-math -std=c99
LFLAGS = -lm -lSDL2 -lSDL2_image -ljson-c
TARGET = iso

build:
	$(CC) $(CFLAGS_BUILD) $(CFLAGS) $(LFLAGS) $(OBJS) -o $(TARGET)

debug:
	$(CC) $(CFLAGS_DEBUG) $(CFLAGS) $(LFLAGS) $(OBJS) -o $(TARGET)

clean:
	rm -f $(TARGET)

test: debug
	./$(TARGET)

CC = gcc
OBJS = src/*.c src/*/*.c
CFLAGS_DEBUG = -Wall -g
CFLAGS_BUILD = -O3
CFLAGS = -ffast-math
LINKER_FLAGS = -lm -lSDL2 -lSDL2_image -ljson-c
TARGET = iso

build:
	$(CC) $(CFLAGS_BUILD) $(CFLAGS) $(LINKER_FLAGS) -o $(TARGET) $(OBJS)

debug:
	$(CC) $(CFLAGS_DEBUG) $(CFLAGS) $(LINKER_FLAGS) -o $(TARGET) $(OBJS)

clean:
	rm -f $(TARGET)

test: debug
	./$(TARGET)

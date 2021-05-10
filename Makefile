OBJS = src/*.c src/*/*.c
COMPILER_FLAGS_DEBUG = -Wall -g -ffast-math
COMPILER_FLAGS_BUILD = -O3 -ffast-math
LINKER_FLAGS = -lm -lSDL2 -lSDL2_image -ljson-c
TARGET = iso

debug:
	gcc $(COMPILER_FLAGS_DEBUG) $(LINKER_FLAGS) -o $(TARGET) $(OBJS)

build:
	gcc $(COMPILER_FLAGS_BUILD) $(LINKER_FLAGS) -o $(TARGET) $(OBJS)

clean:
	rm -f $(TARGET)

test: debug
	./$(TARGET)

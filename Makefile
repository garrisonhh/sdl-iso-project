OBJS = *.c
COMPILER_FLAGS = -Wall -g
LINKER_FLAGS = -lm -lSDL2 -lSDL2_image -ljson-c
TARGET = iso

all:
	gcc $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(TARGET) $(OBJS)

clean:
	rm -f $(TARGET)

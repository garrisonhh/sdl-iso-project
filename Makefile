OBJS = main.c vector.c world.c render.c expose.c
COMPILER_FLAGS = -Wall -g
LINKER_FLAGS = -lm -lSDL2 -lSDL2_image
TARGET = iso

all:
	gcc $(OBJS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(TARGET)

clean:
	rm -f $(TARGET)

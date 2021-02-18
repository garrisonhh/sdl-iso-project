OBJS = *.c
COMPILER_FLAGS = -Wall -g
LINKER_FLAGS = -lm -lSDL2 -lSDL2_image -ljson-c
TARGET = iso

all: compile run

compile:
	gcc $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(TARGET) $(OBJS)

run:
	./$(TARGET)

clean:
	rm -f $(TARGET)

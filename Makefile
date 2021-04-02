OBJS = src/*.c src/*/*.c
COMPILER_FLAGS = -Wall -g
LINKER_FLAGS = -lm -lSDL2 -lSDL2_image -ljson-c
TARGET = iso

compile:
	gcc $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(TARGET) $(OBJS)

run:
	./$(TARGET)

clean:
	rm -f $(TARGET)

test: compile run

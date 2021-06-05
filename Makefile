CONFIG=debug

all: compile

compile:
	cd build/ && premake5 gmake2 && make config=$(CONFIG)

release: 
	CONFIG=release
	make compile

clean:
	cd build/ && make clean


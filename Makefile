CONFIG=debug

all: compile

premake:
	cd build/ && premake5 gmake2

compile:
	cd build/ && make config=$(CONFIG)

release: 
	CONFIG=release
	make compile

clean:
	cd build/ && make clean


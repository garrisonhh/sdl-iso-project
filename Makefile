all: debug

premake:
	cd build/ && premake5 gmake2

debug:
	cd build/ && make config=debug

release:
	cd build/ && make config=release

clean:
	cd build/ && make clean

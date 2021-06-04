# unnamed project

a sandbox game inspired by dwarf fortress and gnomoria with a focus on RPG realism.

![screenshot](/assets/screenshots/game.png)

## getting started

this project is built using [premake](https://premake.github.io/) with gcc and make.

### dependencies
- [SDL2](https://libsdl.org/)
- [SDL2_image](https://www.libsdl.org/projects/SDL_image/)
- [json-c](https://json-c.github.io/json-c/json-c-current-release/doc/html/index.html)

### linux

all dependencies can be found on the major package managers.

### windows

[mingw](https://osdn.net/projects/mingw/) is required.

1. download and install the dependencies
2. make sure the library `include/` and `libs/` directories of all of the libraries are discoverable to gcc. I did this by dropping my include and libs folders into the mingw `include/` and `libs/` folders, and then setting the environment variables `C_INCLUDE_PATH` and `LIBRARY_PATH` to the mingw folder paths.

### build

using bash or powershell, cd to wherever you would like to store the project folder and run the following:

```bash
git clone https://github.com/garrisonhh/sdl-iso-project.git
cd sdl-iso-project/build
premake5 gmake2

# if you're using windows with mingw, replace make with mingw32-make
make config=release
```

the executable will now be generated in the root folder of the project. make sure to run it from the root directory, or the assets will not load properly.

## sources

some of the algorithms I wrote were adapted from (or at least very heavily inspired by):
- [entity collision](https://www.youtube.com/watch?v=8JJ-4JgR7Dg)
- [voxel traversal](https://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.42.3443&rep=rep1&type=pdf)
- [line-sphere intersection](https://gamedev.stackexchange.com/questions/27755/optimized-algorithm-for-line-sphere-intersection-in-glsl)
- [fast circle drawing](https://citeseerx.ist.psu.edu/viewdoc/summary?doi=10.1.1.92.9663)

<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="Creative Commons License" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a>

*to view 4-space tabs (like I use in editing), append "?ts=4" to url of a page*

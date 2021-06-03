# unnamed project

a sandbox game inspired by dwarf fortress and gnomoria with a focus on RPG realism.

![screenshot](/assets/screenshots/game.png)

## getting started

this project is built using premake. to compile, `cd build/` and then `premake5` with your preferred target. I use gnu make, so the `gmake2` target is most likely to work. an executable named `Iso` will be generated in the root folder of the project.

## dependencies

- SDL2
- SDL2\_Image
- json-c

## sources

some of the algorithms I wrote were adapted from (or at least very heavily inspired by):
- [entity collision](https://www.youtube.com/watch?v=8JJ-4JgR7Dg)
- [voxel traversal](https://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.42.3443&rep=rep1&type=pdf)
- [line-sphere intersection](https://gamedev.stackexchange.com/questions/27755/optimized-algorithm-for-line-sphere-intersection-in-glsl)
- [fast circle drawing](https://citeseerx.ist.psu.edu/viewdoc/summary?doi=10.1.1.92.9663)

<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="Creative Commons License" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a>

*to view 4-space tabs (like I use in editing), append "?ts=4" to url of a page*

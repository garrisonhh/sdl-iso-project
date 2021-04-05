## TODO list

#### first priorities
- documentation and quality of life
	- this project is now over two months old and almost 2.5k lines, and so is reaching the critical mass where I'm starting to treat old parts of the code like a black box. this hasn't become a big issue so far, but it will
		- the abstract data structures especially need a guide to their interfaces
		- I don't need to write a detailed paragraph for every function everywhere, but quirks of usage (like deep destruction or avoiding hash key deallocation) can cause obtuse and time consuming bugs
	- code standards are also something I need to more strongly enforce
		- nothing huge, just stuff like more descriptive headers and outlining function/variable naming and stuff like that
- smart entities
	- a\*
		- I think I should really consider some specialized data structures for this after writing a prototype and the issues I encountered
			- binary-tree sets and priority sets specifically look promising
		- node graphs pre-computed and updated on block_set()
			- specifically looking to avoid worst-case a\* searches which iterate over an entire graph before not finding a path
			- this would also be a perfect use case for a binary-tree set
		- a\* is a hefty algorithm computationally and pathfinding is super important in an NPC-focused game
			- if the optimizations aren't enough (I think they will be?) threading is probably the way to go, it just opens up a whole new can of beans
	- explore ai stuff
- gui + interactivity
	- kind of bundled into one. a goal for this project is avoiding overwhelming, confusing UI in other sandbox games like dwarf fortress, factorio, even minecraft's ui can be confusing at times
		- this means minimalism, but not hiding information. a factor in game design decisions should be avoiding the need for a new menu or UI element in the first place
	- a big part of the game I'm envisioning is AI which can interact with the world in the same way the player can, and vice versa
		- this means believable, well-defined, predictable, interesting systems
		- this also means actions need to take place visually, within the world, rather than inside a GUI

#### second priorities
- fluids (water and steam are going to be big huge important game mechanics)

#### ideas
- names
	- clockwork people
- content
	- wildlife
	- fleshing out procgen
		- natural structures
	- other creatures
		- people who live in artificial structures
- tech for whenever
	- dynamic block lighting
	- sprite animations

#### character/art stuff:
- "the animator's survival kit"
- don't think about everything at once
	- design -> animating
- mood board
	- art you wanna take elements from
- sketch
	- keyframes

#### sources
algorithms adapted from (or heavily inspired by) these sources
- [entity collision](https://www.youtube.com/watch?v=8JJ-4JgR7Dg)
- [voxel traversal](https://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.42.3443&rep=rep1&type=pdf)
- [line-sphere intersection](https://gamedev.stackexchange.com/questions/27755/optimized-algorithm-for-line-sphere-intersection-in-glsl)

*to view 4-space tabs (like I use in editing), append "?ts=4" to url of a page*

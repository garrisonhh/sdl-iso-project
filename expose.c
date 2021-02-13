#include <stdlib.h>
#include <stdint.h>
#include "world.h"

/*
are you future me or someone else trying to figure out how in the god damned fuck this function works?
here ya go:
foreach block in chunk:
	set index offset to 1
	if the block isn't air:
		iterates through each of the 3 dimensions (x->y->z)
			set bit 0 of mask to whether that face is exposed
			lshift bitmask 1
			multiply the offset by SIZE (which due to the way blocks[] works, goes from x + 1 -> y + 1 -> z + 1)
		store the new bitmask in the block
*/
// TODO dynamically update block exposure based on surrounding block updates
void exposeChunk(chunk_t *chunk) {
	block_t *block;
	block_t *other;
	int i, j, offset;
	Uint8 newMask;
	for (i = 0; i < CHUNK_SIZE; i++) {
		block = chunk->blocks[i];
		if (block != NULL) {
			newMask = 0;
			offset = 1;
			for (j = 0; j < 3; j++) {
				if ((i + offset) / (offset * SIZE) > i / (offset * SIZE)) {
					newMask |= 1;
				} else {
					other = chunk->blocks[i + offset];
					if (other == NULL) {
						newMask |= 1;
					}
				}
				if (j < 2) {
					offset *= SIZE;
					newMask <<= 1;
				}
			}
			block->exposeMask = newMask;
		}
	}
}

void exposeWorld(world_t *world) {
	for (int i = 0; i < world->numChunks; i++) {
		exposeChunk(world->chunks[i]);
	}
}


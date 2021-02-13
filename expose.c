#include <stdlib.h>
#include <stdint.h>
#include "world.h"
#include "textures.h"

void exposeChunk(chunk_t *chunk) {
	for (int index = 0; index < CHUNK_SIZE; index++) {
		if (chunk->blocks[index] != NULL && chunk->blocks[index]->updateExpose) {
			Uint8 newMask = 0;
			int offset = 1; // the index offset of the next block to check
			int nextOffset = SIZE; // the range of current width-height-depths that aren't edges
			while (offset < CHUNK_SIZE) {
				newMask <<= 1;
				if ((index % nextOffset) + offset < nextOffset) {
					if (chunk->blocks[index + offset] == NULL
							|| textures[chunk->blocks[index + offset]->texture]->transparent) {
						newMask |= 1; // block face is exposed
					}
				} else {
					newMask |= 1; // block is on an edge
				}
				offset = nextOffset;
				nextOffset *= SIZE;
			}
			chunk->blocks[index]->exposeMask = newMask;
			chunk->blocks[index]->updateExpose = false;
		}
	}
}

void exposeWorld(world_t *world) {
	for (int i = 0; i < world->numChunks; i++) {
		exposeChunk(world->chunks[i]);
	}
}


#include <stdlib.h>
#include <stdint.h>
#include "world.h"
#include "textures.h"

void expose_chunk(chunk_t *chunk) {
	Uint8 new_mask;
	int index, offset, next_offset;
	for (index = 0; index < CHUNK_SIZE; index++) {
		if (chunk->blocks[index] != NULL && chunk->blocks[index]->expose_update) {
			new_mask = 0;
			offset = 1; // the index offset of the next block to check
			next_offset = SIZE; // the range of current width-height-depths that aren't edges
			while (offset < CHUNK_SIZE) {
				new_mask <<= 1;
				if ((index % next_offset) + offset < next_offset) {
					if (chunk->blocks[index + offset] == NULL
							|| textures[chunk->blocks[index + offset]->texture]->transparent) {
						new_mask |= 1; // block face is exposed
					}
				} else {
					new_mask |= 1; // block is on an edge
				}
				offset = next_offset;
				next_offset *= SIZE;
			}
			chunk->blocks[index]->expose_mask = new_mask;
			chunk->blocks[index]->expose_update = false;
		}
	}
}

void expose_world(world_t *world) {
	for (int i = 0; i < world->num_chunks; i++) {
		expose_chunk(world->chunks[i]);
	}
}


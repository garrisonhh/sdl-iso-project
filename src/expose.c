#include <stdlib.h>
#include <stdint.h>
#include "world.h"
#include "textures.h"
#include "utils.h"

bool block_transparent(block_t *block) {
	return textures[block->texture]->transparent;
}

// TODO dynamic block exposure
// instead of checking every block every frame, update relevant blocks on block_set() calls
// will be a LOT faster i think...
void expose_world(world_t *world) {
	uint8_t new_mask;
	int x, y, z, i;
	v3i loc, other_loc;
	block_t *block, *other_block;

	FOR_XYZ(x, y, z, world->block_size, world->block_size, world->block_size) {
		loc = (v3i){x, y, z};

		if ((block = block_get(world, loc)) != NULL && block->expose_update) {
			if (block_transparent(block)) {
				block->expose_mask = 0x7;
			} else {
				new_mask = 0;

				for (i = 0; i < 3; i++) {
					new_mask <<= 1;
					
					other_loc = loc;
					v3i_set(&other_loc, i, v3i_get(&loc, i) + 1);

					if ((other_block = block_get(world, other_loc)) == NULL || block_transparent(other_block))
						new_mask |= 1;
				}

				block->expose_mask = new_mask;
			}
		}
	}
}

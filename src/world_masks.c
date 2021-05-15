#include <stdbool.h>
#include "world.h"
#include "world_masks.h"
#include "utils.h"
#include "vector.h"
#include "camera.h"

const v3i OUTLINE_TOP_OFFSETS[4] = {
	(v3i){-1,  0,  1},
	(v3i){ 1,  0,  1},
	(v3i){ 0, -1,  1},
	(v3i){ 0,  1,  1}
};
const v3i OUTLINE_BOTTOM = {0, 0, -1};
const v3i OUTLINE_BOT_OFFSETS[4] = {
	(v3i){-1,  0, -1},
	(v3i){ 1,  0, -1},
	(v3i){ 0, -1, -1},
	(v3i){ 0,  1, -1}
};
const v3i OUTLINE_CORNER_OFFSETS[4] = {
	(v3i){-1, -1,  0},
	(v3i){ 1, -1,  0},
	(v3i){-1,  1,  0},
	(v3i){ 1,  1,  0}
};

bool world_block_see_through(world_t *world, v3i loc) {
	block_t *block = world_get(world, loc);

	return block == NULL || block->texture->transparent;
}

void world_update_masks(world_t *world, v3i loc) {
	block_t *block;

	if ((block = world_get(world, loc)) != NULL) {
		int i, j;
		v3i neighbor;

		// expose mask
		unsigned expose_mask = 0x0;

		for (i = 0; i <= 1; ++i) {
			neighbor = loc;

			for (j = 0; j <= 1; ++j) {
				v3i_set(&neighbor, i, v3i_get(&loc, i) + (j ? 1 : -1));

				if (world_block_see_through(world, neighbor))
					BIT_SET_TRUE(expose_mask, (i << 1) | j);
			}
		}

		neighbor = loc;
		++neighbor.z;

		if (world_block_see_through(world, neighbor))
			expose_mask |= 0x10; // +Z bit

		block->expose_mask = expose_mask;

		// connect mask
		// TODO

		if (block->texture->type == TEX_VOXEL) {
			// outline mask
			unsigned outline_mask = 0x0;

			if (BIT_GET(block->expose_mask, 4)) // +Z bit
				for (i = 0; i < 4; ++i)
					if (BIT_GET(block->expose_mask, i))
						if (world_block_see_through(world, v3i_add(loc, OUTLINE_TOP_OFFSETS[i])))
							BIT_SET_TRUE(outline_mask, i);

			if (world_block_see_through(world, v3i_add(loc, OUTLINE_BOTTOM)))
				for (i = 0; i < 4; ++i)
					if (BIT_GET(block->expose_mask, i))
						if (world_block_see_through(world, v3i_add(loc, OUTLINE_BOT_OFFSETS[i])))
							BIT_SET_TRUE(outline_mask, i + 4);

			block->tex_state.outline_mask = outline_mask;
		}
	}
}

// checks if block is exposed at current camera rotation
bool world_exposed(block_t *block) {
	unsigned dir_bit;

	for (int  i = 0; i <= 1; ++i) {
		dir_bit = ((v3i_get(&camera.render_inc, i) > 0) ? 1 : 0);

		if (BIT_GET(block->expose_mask, (i << 1) | dir_bit))
			return true;
	}

	return false;
}

// generates rotated and trimmed voxel mask data for render_voxel_texture
voxel_masks_t world_voxel_masks(block_t *block, v3i loc) {
	int i;
	unsigned dir_bit;
	voxel_masks_t masks = {0x0, 0x0, 0x0};

	// expose and outline
	for (i = 0; i <= 1; ++i) {
		dir_bit = ((v3i_get(&camera.render_inc, i) > 0) ? 1 : 0);

		if (BIT_GET(block->expose_mask, (i << 1) | dir_bit))
			BIT_SET_TRUE(masks.expose, i);

		if (BIT_GET(block->tex_state.outline_mask, (i << 1) | ((~dir_bit) & 1)))
			BIT_SET_TRUE(masks.outline, i);

		/*
		if (BIT_GET(block->tex_state.outline_mask, ((i << 1) + 4) | dir_bit))
			BIT_SET_TRUE(masks.outline, i + 2);
		*/

		if (v3i_get(&loc, i) == v3i_get(&camera.render_end, i))
			BIT_SET_TRUE(masks.dark, i);
	}

	masks.expose |= BIT_GET(block->expose_mask, 4) << 2;

	if (loc.z == (int)camera.pos.z && !BIT_GET(masks.expose, 2))
		masks.dark |= 0x4;

	// swap XY when camera rotation calls for it
	if (camera.rotation & 1) { // rotation == 1 || rotation == 3
		bool swp;

		BIT_SET_SWAP(masks.expose, 0, 1, swp);
		BIT_SET_SWAP(masks.outline, 0, 1, swp);
		//BIT_SET_SWAP(masks.outline, 2, 3, swp);
		BIT_SET_SWAP(masks.dark, 0, 1, swp);
	} 

	return masks;
}

#include <stdbool.h>
#include "world.h"
#include "world_masks.h"
#include "utils.h"
#include "vector.h"
#include "camera.h"

const int EXPOSE_ROT_BITS[4][3] = {
	{1, 3, 4},
	{3, 0, 4},
	{0, 2, 4},
	{2, 1, 4}
};
const int OUTLINE_ROT_BITS[4][6] = {
	{0, 2, 7, 5, 10, 9},
	{2, 1, 4, 7, 8, 11},
	{1, 3, 6, 4, 9, 10},
	{3, 0, 5, 6, 11, 8},
};

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

bool world_blocks_connect(block_t *block, block_t *other) {
	if (!block->texture->num_tags || !other->texture->num_tags)
		return false;

	int i, j;

	for (i = 0; i < block->texture->num_tags; ++i)
		for (j = 0; j < other->texture->num_tags; ++j)
			if (block->texture->tags[i] == other->texture->tags[j])
				return true;

	return false;
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
				v3i_IDX(neighbor, i) = v3i_IDX(loc, i) + (j ? 1 : -1);

				if (world_block_see_through(world, neighbor))
					BIT_SET_TRUE(expose_mask, (i << 1) | j);
			}
		}

		neighbor = loc;
		++neighbor.z;

		if (world_block_see_through(world, neighbor))
			expose_mask |= 0x10; // +Z bit

		block->expose_mask = expose_mask;

		if (block->texture->type == TEX_CONNECTED) {
			// connected mask
			unsigned connected_mask = 0x0;
			block_t *other;

			for (i = 0; i < 3; ++i) {
				neighbor = loc;

				for (j = 0; j <= 1; ++j) {
					v3i_IDX(neighbor, i) = v3i_IDX(loc, i) + (j ? 1 : -1);

					if ((other = world_get(world, neighbor)) != NULL)
						if (world_blocks_connect(block, other))
							BIT_SET_TRUE(connected_mask, (i << 1) | j);
				}
			}

			block->tex_state.connected_mask = connected_mask;
		} else if (block->texture->type == TEX_VOXEL) {
			// outline mask
			unsigned outline_mask = 0x0;
			bool corner;

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

			for (i = 0; i < 4; ++i) {
				corner = true;

				for (j = 0; j < 3; ++j) {
					neighbor = v3i_add(loc, OUTLINE_CORNER_OFFSETS[i]);

					if (j & 0x1)
						neighbor.x = loc.x;
					else if (j & 0x2)
						neighbor.y = loc.y;

					if (!world_block_see_through(world, neighbor)) {
						corner = false;
						break;
					}
				}

				if (corner)
					BIT_SET_TRUE(outline_mask, i + 8);
			}

			block->tex_state.outline_mask = outline_mask;
		}
	}
}

// checks if block is exposed at current camera rotation
bool world_exposed(block_t *block) {
	for (int i = 0; i < 3; ++i) {
		if (BIT_GET(block->expose_mask, EXPOSE_ROT_BITS[camera.rotation][i]))
			return true;
	}

	return false;
}

// generates rotated and trimmed voxel mask data for render_voxel_texture
voxel_masks_t world_voxel_masks(block_t *block, v3i loc) {
	int i;
	voxel_masks_t masks = {0x0, 0x0, 0x0};

	// expose
	for (i = 0; i < 3; ++i)
		if (BIT_GET(block->expose_mask, EXPOSE_ROT_BITS[camera.rotation][i]))
			BIT_SET_TRUE(masks.expose, i);

	// outline
	for (i = 0; i < 6; ++i)
		if (BIT_GET(block->tex_state.outline_mask, OUTLINE_ROT_BITS[camera.rotation][i]))
			BIT_SET_TRUE(masks.outline, i);

	// dark
	for (i = 0; i < 3; ++i)
		if (v3i_IDX(loc, i) == (v3i_IDX(camera.facing, i) > 0 ? 0 : camera.block_size - 1))
			BIT_SET_TRUE(masks.dark, i);

	if (loc.z == (int)camera.pos.z && !BIT_GET(masks.expose, 2))
		masks.dark |= 0x4;

	if (camera.rotation & 1) {
		bool swp;
		BIT_SET_SWAP(masks.dark, 0, 1, swp);
	}

	return masks;
}

unsigned world_connected_mask(block_t *block) {
	unsigned connected_mask = block->tex_state.connected_mask;
	bool swp;

	switch (camera.rotation) {
	case 1:
		BIT_SET_SWAP(connected_mask, 0, 2, swp);
		BIT_SET_SWAP(connected_mask, 1, 3, swp);
		BIT_SET_SWAP(connected_mask, 2, 3, swp);
		break;
	case 2:
		BIT_SET_SWAP(connected_mask, 0, 1, swp);
		BIT_SET_SWAP(connected_mask, 2, 3, swp);
		break;
	case 3:
		BIT_SET_SWAP(connected_mask, 0, 2, swp);
		BIT_SET_SWAP(connected_mask, 1, 3, swp);
		BIT_SET_SWAP(connected_mask, 0, 1, swp);
		break;
	}

	return connected_mask;
}

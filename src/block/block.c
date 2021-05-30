#include <stdlib.h>
#include "block.h"
#include "blocks.h"
#include "plant.h"
#include "../lib/vector.h"
#include "../world.h"
#include "../camera.h"

block_t *block_create(size_t block_id) {
	block_t *block = malloc(sizeof(block_t));

	*block = *blocks_get(block_id);

	return block;
}

void block_destroy(block_t *block) {
	free(block);
}

void block_tick(block_t *block, world_t *world, double time) {
	switch (block->type) {
	case BLOCK_PLANT:
		plant_tick(&block->state.plant, time);
		block->tex_state.cell.x = plant_growth_level(&block->state.plant);
		break;
	default:
		break;
	}
}

v2i block_project(v3i loc) {
	// modify loc so that it is the back center corner of voxel from camera perspective
	switch (camera.rotation) {
	case 1:
		++loc.x;
		break;
	case 2:
		++loc.x;
		++loc.y;
		break;
	case 3:
		++loc.y;
		break;
	}

	return project_v3i(loc);
}

void block_add_render_info(array_t *packets, block_t *block, v3i loc) {
	render_packet_t *packet = NULL;

	if (block->texture->type == TEX_VOXEL) {
		voxel_masks_t voxel_masks = world_voxel_masks(block, loc);

		if (voxel_masks.expose || voxel_masks.dark) {
			packet = render_texture_packet_create(loc, block_project(loc), block->texture);
			packet->texture.state.voxel_masks = voxel_masks;
		}
	} else if (world_exposed(block)) {
		packet = render_texture_packet_create(loc, block_project(loc), block->texture);

		if (block->texture->type == TEX_CONNECTED)
			packet->texture.state.connected_mask = world_connected_mask(block);
		else
			packet->texture.state.tex = block->tex_state;
	} else if (!block->texture->transparent) {
		if (block->expose_mask) {
			packet = render_texture_packet_create(loc, block_project(loc), block->texture);

			if (block->texture->type == TEX_CONNECTED)
				packet->texture.state.connected_mask = world_connected_mask(block);
			else
				packet->texture.state.tex = block->tex_state;
		} else {
			packet = render_texture_packet_create(loc, block_project(loc), DARK_VOXEL_TEXTURE);
			packet->texture.state.voxel_masks = world_voxel_masks(block, loc);
		}
	}

	if (packet != NULL)
		array_push(packets, packet);
}

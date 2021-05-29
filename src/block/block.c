#include <stdlib.h>
#include "block.h"
#include "blocks.h"
#include "plant.h"
#include "../vector.h"
#include "../world.h"

block_t *block_create(size_t block_id) {
	block_t *block = malloc(sizeof(block_t));

	*block = *block_gen_get(block_id);

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

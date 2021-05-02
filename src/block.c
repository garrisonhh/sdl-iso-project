#include <stdlib.h>
#include "block.h"
#include "block_gen.h"
#include "vector.h"
#include "world.h"
#include "block_types/plant.h"

block_t *block_create(size_t block_id) {
	block_t *block = (block_t *)malloc(sizeof(block_t));

	*block = *block_gen_get(block_id);

	// TODO TESTING REMOVE
	if (block->type == BLOCK_PLANT) {
		block->state.plant = (plant_t){
			.growth = 0.0,
			.growth_rate = 0.01,
			.growth_rate_delta = 0.005,
			.fullgrown = 2
		};
	}

	return block;
}

void block_destroy(block_t *block) {
	free(block);
}

void block_tick(block_t *block, world_t *world, double time) {
	switch (block->type) {
		case BLOCK_PLANT:
			plant_tick(&block->state.plant, time);
			block->tex_state.state.cell.x = plant_growth_level(&block->state.plant);
			break;
		default:
			break;
	}
}

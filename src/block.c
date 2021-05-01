#include <stdlib.h>
#include "block.h"
#include "block_gen.h"
#include "vector.h"
#include "world.h"
#include "block_types/plant.h"

block_t *block_create(size_t block_id) {
	block_t *block = (block_t *)malloc(sizeof(block_t));

	*block = *block_gen_get(block_id);

	return block;
}

void block_destroy(block_t *block) {
	free(block);
}

void block_tick(block_t *block, world_t *world) {
	// TODO
}

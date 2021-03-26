#include <stdlib.h>
#include <stdint.h>
#include "block.h"

block_t *block_create(size_t texture) {
	block_t *block = (block_t *)malloc(sizeof(block_t));

	block->texture = texture;
	block->expose_mask = 0x7;
	block->connect_mask = 0x0;

	return block;
}

void block_destroy(block_t *block) {
	free(block);
}

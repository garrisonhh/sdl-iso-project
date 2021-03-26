#ifndef BLOCK_H
#define BLOCK_H

#include <stdlib.h>
#include <stdint.h>

/* 
 * TODO texture data, masks etc into a 'tex_data_t' struct for better encapsulation
 */
struct block_t {
	size_t texture;
	uint8_t expose_mask; // last 3 bits are X Y Z
	uint8_t connect_mask; // last 6 bits are +X -X +Y -Y +Z -Z
};
typedef struct block_t block_t;

block_t *block_create(size_t texture);
void block_destroy(block_t *block);

#endif

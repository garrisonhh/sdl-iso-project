#ifndef WORLD_MASKS_H
#define WORLD_MASKS_H

#include "vector.h"

typedef struct world_t world_t;
typedef struct block_t block_t;

struct voxel_masks_t {
	unsigned expose: 3;
	unsigned outline: 6;
	unsigned dark: 3;
};
typedef struct voxel_masks_t voxel_masks_t;

void world_update_masks(world_t *world, v3i loc);
bool world_exposed(block_t *block);
voxel_masks_t world_voxel_masks(block_t *block, v3i loc);
unsigned world_connected_mask(block_t *block);

#endif

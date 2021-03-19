#include <stdio.h>
#include "entity.h"
#include "vector.h"
#include "collision.h"
#include "world.h"
#include "list.h"

void entity_destroy(entity_t *entity) {
	free(entity);
}

// TODO int -> size_t for conventions
// TODO entities larger than 3x3 (or 1x1? unsure what is the largest supported)
// list_deep_destroy() when done with data
list_t *entity_surrounding_bboxes(entity_t *entity, world_t *world) {
	list_t *boxes;
	v3i entity_loc, current_block;
	bbox_t *block_box;
	int x, y, z;
	unsigned int chunk_index, block_index;

	boxes = list_create();
	entity_loc = v3i_from_v3d(entity->ray.pos);
	
	for (z = -1; z <= 1; z++) {
		for (y = -1; y <= 1; y++) {
			for (x = -1; x <= 1; x++) {
				current_block = (v3i){x, y, z};
				current_block = v3i_add(entity_loc, current_block);

				if (!chunk_block_indices(world, current_block, &chunk_index, &block_index)
				 || world->chunks[chunk_index]->blocks[block_index] != NULL) {
					block_box = (bbox_t *)malloc(sizeof(bbox_t));

					block_box->pos = v3d_sub(v3d_from_v3i(current_block), entity->center);
	 				block_box->size = v3d_add(BLOCK_SIZE, entity->size);

					list_add(boxes, block_box);
				}
			}
		}
	}

	return boxes;
}

void entity_tick(entity_t *entity, struct world_t *world, int ms) {
	list_t *boxes;
	ray_t scaled_ray;
	v3d resolved_dir;
	double time;
	int axis;

	time = (double)ms / 1000;
	entity->ray.dir.z += GRAVITY * time;

	scaled_ray = entity->ray;
	scaled_ray.dir = v3d_scale(scaled_ray.dir, time);

	// collision resolution
	boxes = entity_surrounding_bboxes(entity, world);
	sort_bboxes_by_vector_polarity(boxes, scaled_ray.dir);

	for (size_t i = 0; i < boxes->size; i++) {
		if ((axis = ray_bbox_intersection(scaled_ray, *(bbox_t *)boxes->items[i], NULL, &resolved_dir)) >= 0) {
			scaled_ray.dir = resolved_dir;

			// kill inertia on collided axes
			// TODO on_ground flag or similar for entities
			v3d_set(&entity->ray.dir, axis, 0);
		}
	}

	list_deep_destroy(boxes);

	entity->ray.pos = v3d_add(entity->ray.pos, scaled_ray.dir);
}

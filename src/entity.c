#include <stdio.h>
#include <stdbool.h>
#include "entity.h"
#include "vector.h"
#include "collision.h"
#include "world.h"
#include "block.h"
#include "list.h"

entity_t *entity_create(size_t sprite, v3d pos, v3d size) {
	entity_t *entity = (entity_t *)malloc(sizeof(entity_t));

	entity->sprite = sprite;
	entity->ray = (ray_t){pos, (v3d){0.0, 0.0, 0.0}};
	entity->size = size;
	entity->center = v3d_scale(entity->size, 0.5);
	entity->on_ground = false;

	return entity;
}

void entity_destroy(entity_t *entity) {
	free(entity);
}

/*
 * TODO entities larger than 3x3 (or 1x1? unsure what is the largest supported)
 * TODO support speeds of more than 1 block per frame
 * both can be solved by making a box around the entity of size entity.size, and then combining
 * that with the shape of the scaled_ray, and then checking any blocks which might collide with
 * that box.
 */
list_t *entity_surrounding_block_colls(entity_t *entity, world_t *world) {
	list_t *block_colls;
	v3i entity_loc, current_loc;
	block_t *block;
	block_collidable_t *block_coll;
	int x, y, z;
	unsigned int chunk_index, block_index;

	block_colls = list_create();
	entity_loc = v3i_from_v3d(entity->ray.pos);

	size_t test = 0;
	
	for (z = -1; z <= 1; z++) {
		for (y = -1; y <= 1; y++) {
			for (x = -1; x <= 1; x++) {
				current_loc = (v3i){x, y, z};
				current_loc = v3i_add(entity_loc, current_loc);

				if (chunk_block_indices(world, current_loc, &chunk_index, &block_index)
				 && (block = world->chunks[chunk_index]->blocks[block_index]) != NULL
				 && block->coll_data->coll_type != BLOCK_COLL_NONE) {
					block_coll = (block_collidable_t *)malloc(sizeof(block_collidable_t));

					block_coll->loc = current_loc;
					block_coll->coll_data = block->coll_data;

					list_add(block_colls, block_coll);
					test++;
				}
			}
		}
	}

	return block_colls;
}

void entity_tick(entity_t *entity, struct world_t *world, double ms) {
	double time;
	int axis;
	v3d resolved_dir;
	ray_t scaled_ray;
	bbox_t block_bbox;
	block_collidable_t *block_coll;
	list_t *block_colls;

	time = ms / 1000;
	entity->ray.dir.z += GRAVITY * time;
	entity->on_ground = false;

	scaled_ray = entity->ray;
	scaled_ray.dir = v3d_scale(scaled_ray.dir, time);

	block_colls = entity_surrounding_block_colls(entity, world);
	block_coll_list_sort(block_colls, scaled_ray.dir);

	for (size_t i = 0; i < block_colls->size; i++) {
		block_coll = (block_collidable_t *)block_colls->items[i];
		block_bbox = *block_coll->coll_data->bbox;

		block_bbox.pos = v3d_add(block_bbox.pos, v3d_from_v3i(block_coll->loc));
		block_bbox.pos = v3d_sub(block_bbox.pos, entity->center);
		block_bbox.size = v3d_add(block_bbox.size, entity->size);

		if ((axis = ray_bbox_intersection(scaled_ray, block_bbox, NULL, &resolved_dir)) >= 0) {
			scaled_ray.dir = resolved_dir;

			v3d_set(&entity->ray.dir, axis, 0);

			if (axis == 2)
				entity->on_ground = true;
		}
	}

	entity->ray.pos = v3d_add(entity->ray.pos, scaled_ray.dir);

	list_deep_destroy(block_colls);
}

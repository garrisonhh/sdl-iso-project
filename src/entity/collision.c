#include "collision.h"
#include <ghh/utils.h>
#include "../block/blocks.h"

array_t *entity_surrounding_block_colls(entity_t *entity, world_t *world) {
	v3i entity_loc, loc, offset;
	array_t *block_colls;
	block_t *block;
	block_collidable_t *block_coll;

	block_colls = array_create(27);
	entity_loc = v3i_from_v3d(entity->data.ray.pos);

	FOR_CUBE(offset.x, offset.y, offset.z, -1, 2) {
		loc = v3i_add(entity_loc, offset);

		if ((block = world_get(world, loc)) != NULL
		  && block->coll_data->coll_type != BLOCK_COLL_NONE) {
			block_coll = malloc(sizeof(block_collidable_t));

			block_coll->loc = loc;
			block_coll->coll_data = block->coll_data;

			array_push(block_colls, block_coll);
		} else if (loc.x < 0 || loc.x >= world->block_size
				|| loc.y < 0 || loc.y >= world->block_size
				|| loc.z < 0 || loc.z >= world->block_size) {
			block_coll = malloc(sizeof(block_collidable_t));

			block_coll->loc = loc;
			block_coll->coll_data = &WALL_COLL_DATA;

			array_push(block_colls, block_coll);
		}
	}

	return block_colls;
}

// returns resolved entity movement ray
ray_t entity_collide_bbox(entity_t *entity, ray_t movement, bbox_t block_bbox) {
	int collision_axis;
	v3d resolved_dir;

	collision_axis = ray_intersects_bbox(movement, block_bbox, NULL, &resolved_dir);

	if (collision_axis >= 0) {
		v3d_IDX(entity->data.ray.dir, collision_axis) = 0;

		if (collision_axis == 2 && (movement.dir.z <= 0.0 || fequals(movement.dir.z, 0)))
			entity->data.on_ground = true;

		movement.dir = resolved_dir;
	}

	return movement;
}

void entity_move_and_collide(entity_t *entity, world_t *world, double time) {
	ray_t movement;
	bbox_t block_bbox;
	block_collidable_t *block_coll;
	array_t *block_colls;

	movement = entity->data.ray;
	movement.dir = v3d_scale(movement.dir, time);

	block_colls = entity_surrounding_block_colls(entity, world);
	block_coll_array_sort(block_colls, entity->data.ray.dir);

	entity->data.on_ground = false;

	for (size_t i = 0; i < array_size(block_colls); i++) {
		block_coll = array_get(block_colls, i);

		// add entity size to block bbox size
		block_bbox = *block_coll->coll_data->bbox;
		block_bbox.pos = v3d_add(block_bbox.pos, v3d_from_v3i(block_coll->loc));
		block_bbox.pos = v3d_sub(block_bbox.pos, entity->data.center);
		block_bbox.size = v3d_add(block_bbox.size, entity->data.size);

		if (block_coll->coll_data->coll_type != BLOCK_COLL_NONE)
			movement = entity_collide_bbox(entity, movement, block_bbox);
	}

	entity->data.ray.pos = v3d_add(entity->data.ray.pos, movement.dir);

	array_destroy(block_colls, true);
}

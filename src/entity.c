#include <stdio.h>
#include <stdbool.h>
#include "entity.h"
#include "vector.h"
#include "collision.h"
#include "world.h"
#include "block.h"
#include "block_gen.h"
#include "data_structures/array.h"
#include "data_structures/list.h"
#include "utils.h"

entity_t *entity_create(texture_t *sprite, v3d pos, v3d size) {
	entity_t *entity = (entity_t *)malloc(sizeof(entity_t));

	entity->sprite = sprite;
	entity->ray = (ray_t){pos, (v3d){0.0, 0.0, 0.0}};
	entity->size = size;
	entity->center = v3d_scale(entity->size, 0.5);

	entity->on_ground = false;
	entity->path = list_create();

	return entity;
}

void entity_destroy(entity_t *entity) {
	list_destroy(entity->path, true);

	free(entity);
}

/*
 * TODO entities larger than 1x1
 * TODO support speeds of more than 1 block per frame
 * both can be solved by making a box around the entity of size entity.size, and then combining
 * that with the shape of the scaled_ray, and then checking any blocks which might collide with
 * that box.
 */
array_t *entity_surrounding_block_colls(entity_t *entity, world_t *world) {
	int x, y, z;
	v3i entity_loc, current_loc;
	array_t *block_colls;
	block_t *block;
	block_collidable_t *block_coll;

	block_colls = array_create(27);
	entity_loc = v3i_from_v3d(entity->ray.pos);

	for (z = -1; z <= 1; z++) {
		for (y = -1; y <= 1; y++) {
			for (x = -1; x <= 1; x++) {
				current_loc = (v3i){x, y, z};
				current_loc = v3i_add(entity_loc, current_loc);

				if ((block = block_get(world, current_loc)) != NULL
				  && block->coll_data->coll_type != BLOCK_COLL_NONE) {
					block_coll = (block_collidable_t *)malloc(sizeof(block_collidable_t));

					block_coll->loc = current_loc;
					block_coll->coll_data = block->coll_data;

					array_add(block_colls, block_coll);
				} else if (current_loc.x < 0 || current_loc.x >= world->block_size
						|| current_loc.y < 0 || current_loc.y >= world->block_size
						|| current_loc.z < 0 || current_loc.z >= world->block_size) {
					block_coll = (block_collidable_t *)malloc(sizeof(block_collidable_t));

					block_coll->loc = current_loc;
					block_coll->coll_data = &WALL_COLL_DATA;

					array_add(block_colls, block_coll);
				}
			}
		}
	}

	return block_colls;
}

// TODO this is absolutely unreadable and extremely messy. figure out how to split it up.
void entity_move_and_collide(entity_t *entity, array_t *block_colls, double time) {
	int bbox_coll_axis;
	bool intersecting, behind, check_bbox;
	ray_t scaled_ray, block_plane;
	v3d resolved_dir, intersect;
	bbox_t block_bbox;
	block_collidable_t *block_coll;
	block_coll_e coll_type;

	scaled_ray = entity->ray;
	scaled_ray.dir = v3d_scale(scaled_ray.dir, time);

	block_coll_array_sort(block_colls, entity->ray.dir);

	for (size_t i = 0; i < block_colls->size; i++) {
		block_coll = (block_collidable_t *)block_colls->items[i];
		coll_type = block_coll->coll_data->coll_type;

		block_bbox = *block_coll->coll_data->bbox;
		block_bbox.pos = v3d_add(block_bbox.pos, v3d_from_v3i(block_coll->loc));
		block_bbox.pos = v3d_sub(block_bbox.pos, entity->center);
		block_bbox.size = v3d_add(block_bbox.size, entity->size);

		check_bbox = false;

		if (coll_type == BLOCK_COLL_CHOPPED_BOX) {
			block_plane = *block_coll->coll_data->plane;
			block_plane.pos = v3d_add(block_plane.pos, v3d_from_v3i(block_coll->loc));
			block_plane.pos = v3d_add(block_plane.pos, v3d_mul(entity->center, v3d_from_v3i(polarity_of_v3d(block_plane.dir))));


			intersecting = ray_intersects_plane(scaled_ray, block_plane, &intersect, &resolved_dir, &behind);

			if (behind) {
				if (intersecting && inside_bbox(block_bbox, intersect)) { // ray collides with plane inside the box
					scaled_ray.dir = resolved_dir;

					// v3d_print("resolving to", resolved_dir);
					//v3d_print("(plane) scaled_ray.dir", scaled_ray.dir);

					// TODO friction?/reduce values based on difference from normal
					if (block_coll->coll_data->plane->dir.z > 0) {
						entity->ray.dir.z = 0;
					}
				} else { // ray may collide with box but not on the face of the plane
					check_bbox = true;
				}
			}
		} else {
			check_bbox = true;
		}

		if (check_bbox && (bbox_coll_axis = ray_intersects_bbox(scaled_ray, block_bbox, NULL, &resolved_dir)) >= 0) {
			scaled_ray.dir = resolved_dir;

			if (coll_type == BLOCK_COLL_CHOPPED_BOX) {
				// v3d_print("resolving to", resolved_dir);
				//v3d_print("(bbox)  scaled_ray.dir", scaled_ray.dir);
			}

			v3d_set(&entity->ray.dir, bbox_coll_axis, 0);

			if (bbox_coll_axis == 2)
				entity->on_ground = true;
		}
	}

	entity->ray.pos = v3d_add(entity->ray.pos, scaled_ray.dir);
}

void entity_add_path(entity_t *entity, list_t *path) {
	list_merge(entity->path, path);
	list_destroy(path, false);
}

// TODO make these part of entity_t struct
// leaving them here just to get a working version
#define ENTITY_XY_SPEED 6.0
#define ENTITY_JUMP 9.0
#define DIST_TOLERANCE 0.01

void entity_follow_path(entity_t *entity, double time) {
	if (entity->path->size == 0)
		return;

	v3d next, diff, dir;
	double dist;

	next = v3d_from_v3i(*(v3i *)list_peek(entity->path));
	next.x += .5;
	next.y += .5;
	next.z += entity->center.z;

	diff = v3d_sub(next, entity->ray.pos);
	dist = v3d_magnitude(diff);

	dir = diff;
	dir.z = 0;
	dir = v3d_scale(v3d_normalize(dir), ENTITY_XY_SPEED);
	dir.z = entity->ray.dir.z;
	
	// TODO some sort of jump() function
	if (diff.z > 0) {
		if (entity->on_ground)
			dir.z += ENTITY_JUMP;
	}

	entity->ray.dir = dir;

	// remove path v3i when close enough
	if (dist < time * v3d_magnitude(entity->ray.dir)) {
		v3i *popping = list_pop(entity->path);
		v3i_print("popping", *popping);
		free(popping);

		//free(list_pop(entity->path));

		if (entity->path->size == 0)
			printf("PATH DONE!\n");
	}
}

void entity_tick(entity_t *entity, struct world_t *world, double ms) {
	double time;
	array_t *block_colls;

	time = ms / 1000;
	time = MIN(time, 0.1); // prevent lag bugs

	// think (modify state)
	entity_follow_path(entity, time);

	// act (apply changes)
	entity->on_ground = false;
	entity->ray.dir.z += GRAVITY * time;

	block_colls = entity_surrounding_block_colls(entity, world);
	entity_move_and_collide(entity, block_colls, time);

	array_destroy(block_colls, true);
}

#include <stdio.h>
#include <stdbool.h>
#include "entity.h"
#include "vector.h"
#include "collision.h"
#include "world.h"
#include "block_gen.h"
#include "block_collision.h"
#include "textures.h"
#include "animation.h"
#include "camera.h"
#include "data_structures/array.h"
#include "data_structures/list.h"
#include "utils.h"

void entity_update_directions(entity_t *);

entity_t *entity_create(sprite_t **sprites, size_t num_sprites, v3d pos, v3d size) {
	entity_t *entity = malloc(sizeof(entity_t));

	entity->sprites = sprites;
	entity->num_sprites = num_sprites;
	entity->anim_states = malloc(sizeof(animation_t) * num_sprites);

	for (size_t i = 0; i < entity->num_sprites; ++i) {
		entity->anim_states[i].cell = (v2i){0, 0};
		entity->anim_states[i].state = 0.0;
	}

	entity->last_dir = (v3d){0.0, 1.0, 0.0};
	entity->dir_xy = DIR_FRONT;
	entity->dir_z = DIR_LEVEL;

	entity->ray = (ray_t){pos, (v3d){0.0, 0.0, 0.0}};
	entity->size = size;
	entity->center = v3d_scale(entity->size, 0.5);

	entity->on_ground = false;
	entity->path = list_create();

	entity_update_directions(entity); // sets dir_xy and dir_z

	return entity;
}

void entity_destroy(entity_t *entity) {
	free(entity->sprites);
	free(entity->anim_states);

	list_destroy(entity->path, true);

	free(entity);
}

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

				if ((block = world_get(world, current_loc)) != NULL
				  && block->coll_data->coll_type != BLOCK_COLL_NONE) {
					block_coll = malloc(sizeof(block_collidable_t));

					block_coll->loc = current_loc;
					block_coll->coll_data = block->coll_data;

					array_add(block_colls, block_coll);
				} else if (current_loc.x < 0 || current_loc.x >= world->block_size
						|| current_loc.y < 0 || current_loc.y >= world->block_size
						|| current_loc.z < 0 || current_loc.z >= world->block_size) {
					block_coll = malloc(sizeof(block_collidable_t));

					block_coll->loc = current_loc;
					block_coll->coll_data = &WALL_COLL_DATA;

					array_add(block_colls, block_coll);
				}
			}
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
		v3d_set(&entity->ray.dir, collision_axis, 0);

		if (collision_axis == 2 && (movement.dir.z <= 0.0 || d_close(movement.dir.z, 0)))
			entity->on_ground = true;

		movement.dir = resolved_dir;
	}

	return movement;
}

// returns resolved entity movement ray
// NOTE: chopped box collision is still wonky at best, while it is super cool. idk whether
// it actually makes sense in the scope of alpha 1.0 at least, it is consuming more time than I want it
// to and generally poses a lot of technical challenges in a lot of areas. future versions, sure lets
// come back to the idea
ray_t entity_collide_chopped_bbox(entity_t *entity, ray_t movement, bbox_t block_bbox, block_collidable_t *block_coll) {
	ray_t plane;
	v3d intersect, resolved_dir;
	bool behind_plane, intersects_plane;

	plane = *block_coll->coll_data->plane;
	plane.pos = v3d_add(plane.pos, v3d_from_v3i(block_coll->loc));
	plane.pos = v3d_add(plane.pos, v3d_mul(entity->center, v3d_from_v3i(polarity_of_v3d(plane.dir))));

	intersects_plane = ray_intersects_plane(movement, plane, &intersect, &resolved_dir, &behind_plane);

	if (behind_plane) {
		if (intersects_plane && inside_bbox(block_bbox, intersect)) { // ray collides with plane inside the box
			if (plane.dir.z > 0 && !d_close(plane.dir.z, 0)) { // plane is facing upwards (it can be stood upon)
				entity->on_ground = true;

				if (resolved_dir.z <= 0 || d_close(resolved_dir.z, 0)) {
					entity->ray.dir.z = 0;
					resolved_dir.z = 0;
				}
			}

			movement.dir = resolved_dir;
		} else { // ray may collide with box but not on the face of the plane
			movement = entity_collide_bbox(entity, movement, block_bbox);
		}
	}

	return movement;
}

void entity_move_and_collide(entity_t *entity, array_t *block_colls, double time) {
	ray_t movement;
	bbox_t block_bbox;
	block_collidable_t *block_coll;

	movement = entity->ray;
	movement.dir = v3d_scale(movement.dir, time);

	block_coll_array_sort(block_colls, entity->ray.dir);

	for (size_t i = 0; i < block_colls->size; i++) {
		block_coll = (block_collidable_t *)block_colls->items[i];

		// add entity size to block bbox size
		block_bbox = *block_coll->coll_data->bbox;
		block_bbox.pos = v3d_add(block_bbox.pos, v3d_from_v3i(block_coll->loc));
		block_bbox.pos = v3d_sub(block_bbox.pos, entity->center);
		block_bbox.size = v3d_add(block_bbox.size, entity->size);

		switch (block_coll->coll_data->coll_type) {
			case (BLOCK_COLL_CHOPPED_BOX):
				movement = entity_collide_chopped_bbox(entity, movement, block_bbox, block_coll);
				break;
			case (BLOCK_COLL_CUSTOM_BOX):
			case (BLOCK_COLL_DEFAULT_BOX):
				movement = entity_collide_bbox(entity, movement, block_bbox);
				break;
			case (BLOCK_COLL_NONE):
				break;
		}
	}

	entity->ray.pos = v3d_add(entity->ray.pos, movement.dir);
}

void entity_add_path(entity_t *entity, list_t *path) {
	list_merge(entity->path, path);
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
	if (diff.z > 0)
		if (entity->on_ground)
			dir.z += ENTITY_JUMP;

	entity->ray.dir = dir;

	// remove path v3i when close enough
	if (dist < time * v3d_magnitude(entity->ray.dir)) {
		v3i *popping = list_pop(entity->path);
		v3i_print("popping", *popping);
		free(popping);

		if (entity->path->size == 0)
			printf("PATH DONE!\n");
	}
}

double entity_y(entity_t *entity) {
	return entity->ray.pos.x + entity->ray.pos.y;
}

int entity_bucket_compare(const void *a, const void *b) {
	return (((entity_y(*(entity_t **)a) - entity_y(*(entity_t **)b)) >= 0) ? 1 : -1);
}

void entity_update_directions(entity_t *entity) {
	double dir;
	v3i facing;

	for (int i = 0; i < 3; ++i) {
		dir = v3d_get(&entity->last_dir, i);

		if (d_close(dir, 0.0))
			v3i_set(&facing, i, 0);
		else if (dir > 0.0)
			v3i_set(&facing, i, 1);
		else
			v3i_set(&facing, i, -1);
	}

	facing = camera_rotated_v3i(facing);

	if (facing.z < 0)
		entity->dir_z = DIR_DOWN;
	else if (facing.z > 0)
		entity->dir_z = DIR_UP;
	else
		entity->dir_z = DIR_LEVEL;

	if (facing.y < 0) {
		if (facing.x < 0)
			entity->dir_xy = DIR_BACK_LEFT;
		else if (facing.x > 0)
			entity->dir_xy = DIR_BACK_RIGHT;
		else
			entity->dir_xy = DIR_BACK;
	} else if (facing.y > 0) {
		if (facing.x < 0)
			entity->dir_xy = DIR_FRONT_LEFT;
		else if (facing.x > 0)
			entity->dir_xy = DIR_FRONT_RIGHT;
		else
			entity->dir_xy = DIR_FRONT;
	} else {
		if (facing.x < 0)
			entity->dir_xy = DIR_LEFT;
		else if (facing.x > 0)
			entity->dir_xy = DIR_RIGHT;
	}
}

void entity_tick(entity_t *entity, world_t *world, double time) {
	int i;
	array_t *block_colls;
	sprite_t *sprite;
	animation_t *anim_state;

	time = MIN(time, 0.1); // super slow physics ticks means broken physics

	// sprite state
	// sprite 2d direction
	if (!d_close(fabs(entity->ray.dir.x) + fabs(entity->ray.dir.y), 0)) {
		entity->last_dir.x = entity->ray.dir.x - entity->ray.dir.y;
		entity->last_dir.y = entity->ray.dir.x + entity->ray.dir.y;
	}

	entity->last_dir.z = entity->ray.dir.z;
	entity_update_directions(entity);

	// animation state
	for (i = 0; i < entity->num_sprites; ++i) {
		sprite = entity->sprites[i];
		anim_state = &entity->anim_states[i];

		switch (sprite->type) {
			case SPRITE_STATIC:
				break;
			case SPRITE_HUMAN_BODY:
				anim_human_body(entity, anim_state);
				break;
			case SPRITE_HUMAN_BACK_HANDS:
			case SPRITE_HUMAN_FRONT_HANDS:
				anim_human_hands(entity, anim_state);
				break;
		}

		if (sprite->anim_lengths[anim_state->cell.y] > 1) {
			anim_state->state += time * ANIMATION_FPS;

			if (anim_state->state > sprite->anim_lengths[anim_state->cell.y])
				anim_state->state -= (double)sprite->anim_lengths[anim_state->cell.y];

			anim_state->cell.x = (int)anim_state->state;
		}
	}

	// think (modify state)
	entity_follow_path(entity, time);

	// act (apply changes)
	if (!entity->on_ground)
		entity->ray.dir.z += GRAVITY * time;

	entity->on_ground = false;

	block_colls = entity_surrounding_block_colls(entity, world);
	entity_move_and_collide(entity, block_colls, time);
	array_destroy(block_colls, true);
}

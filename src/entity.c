#include <stdio.h>
#include <stdbool.h>
#include "entity.h"
#include "vector.h"
#include "world.h"
#include "block_gen.h"
#include "block_collision.h"
#include "camera.h"
#include "utils.h"
#include "render.h"

void entity_update_directions(entity_t *);
void entity_move_and_collide(entity_t *, world_t *world, double time);

entity_t *entity_create(entity_type_e type, texture_t *sprite, v3d size) {
	entity_t *entity = malloc(sizeof(entity_t));

	entity->type = type;

	switch (entity->type) {
		case ENTITY_HUMAN:
			entity->state.human = human_create();
			break;
		default:
			break;
	}

	entity->sprite = sprite;
	entity->anim_state.cell = (v2i){0, 0};
	entity->anim_state.state = 0.0;

	entity->last_dir = (v3d){0, 1, 0};
	entity->dir_xy = DIR_FRONT;
	entity->dir_z = DIR_LEVEL;

	entity->ray = (ray_t){(v3d){0, 0, 0}, (v3d){0, 0, 0}};
	entity->size = size;
	entity->center = v3d_scale(entity->size, 0.5);

	entity->on_ground = false;

	return entity;
}

void entity_destroy(entity_t *entity) {
	switch (entity->type) {
		case ENTITY_HUMAN:
			human_destroy(entity->state.human);
		default:
			break;
	}

	free(entity);
}

void entity_tick(entity_t *entity, world_t *world, double time) {
	time = MIN(time, 0.1); // super slow physics ticks means broken physics

	// entity state
	switch (entity->type) {
		case ENTITY_BASE:
			break;
		case ENTITY_HUMAN:
			entity_human_tick(entity, time);
			break;
	}

	// sprite/animation state
	anim_entity_update_directions(entity);
	anim_tick(entity, entity->sprite, &entity->anim_state, time);

	// movement
	if (!entity->on_ground)
		entity->ray.dir.z += GRAVITY * time;

	entity_move_and_collide(entity, world, time);
}

v2i entity_screen_pos(entity_t *entity) {
	v3d pos = entity->ray.pos;

	pos.z -= entity->center.z;

	return project_v3d(pos);
}

void entity_add_sprite_packet(array_t *packets, v2i pos, texture_t *sprite, animation_t *anim_state) {
	render_packet_t *packet = render_packet_create(pos, sprite);

	packet->state.anim = *anim_state;

	array_add(packets, packet);
}

void entity_add_render_packets(entity_t *entity, array_t *packets) {
	v2i pos = entity_screen_pos(entity);
	render_packet_t *base_packet = render_packet_create(pos, entity->sprite);
	
	base_packet->state.anim = entity->anim_state;

	switch (entity->type) {
		case ENTITY_BASE:
			array_add(packets, base_packet);

			break;
		case ENTITY_HUMAN:;
			human_t *human = entity->state.human;
			texture_t **sprites;

			if (human->tool == NULL)
				sprites = human->hands;
			else
				sprites = human->tool->sprites;

			entity_add_sprite_packet(packets, pos, sprites[0], &human->anim_state);
			array_add(packets, base_packet);
			entity_add_sprite_packet(packets, pos, sprites[1], &human->anim_state);

			break;
	}
}

array_t *entity_surrounding_block_colls(entity_t *entity, world_t *world) {
	v3i entity_loc, loc, offset;
	array_t *block_colls;
	block_t *block;
	block_collidable_t *block_coll;

	block_colls = array_create(27);
	entity_loc = v3i_from_v3d(entity->ray.pos);

	FOR_CUBE(offset.x, offset.y, offset.z, -1, 2) {
		loc = v3i_add(entity_loc, offset);

		if ((block = world_get(world, loc)) != NULL
		  && block->coll_data->coll_type != BLOCK_COLL_NONE) {
			block_coll = malloc(sizeof(block_collidable_t));

			block_coll->loc = loc;
			block_coll->coll_data = block->coll_data;

			array_add(block_colls, block_coll);
		} else if (loc.x < 0 || loc.x >= world->block_size
				|| loc.y < 0 || loc.y >= world->block_size
				|| loc.z < 0 || loc.z >= world->block_size) {
			block_coll = malloc(sizeof(block_collidable_t));

			block_coll->loc = loc;
			block_coll->coll_data = &WALL_COLL_DATA;

			array_add(block_colls, block_coll);
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
		v3d_IDX(entity->ray.dir, collision_axis) = 0;

		if (collision_axis == 2 && (movement.dir.z <= 0.0 || d_close(movement.dir.z, 0)))
			entity->on_ground = true;

		movement.dir = resolved_dir;
	}

	return movement;
}

void entity_move_and_collide(entity_t *entity, world_t *world, double time) {
	ray_t movement;
	bbox_t block_bbox;
	block_collidable_t *block_coll;
	array_t *block_colls;

	movement = entity->ray;
	movement.dir = v3d_scale(movement.dir, time);

	block_colls = entity_surrounding_block_colls(entity, world);
	block_coll_array_sort(block_colls, entity->ray.dir);

	entity->on_ground = false;

	for (size_t i = 0; i < block_colls->size; i++) {
		block_coll = (block_collidable_t *)block_colls->items[i];

		// add entity size to block bbox size
		block_bbox = *block_coll->coll_data->bbox;
		block_bbox.pos = v3d_add(block_bbox.pos, v3d_from_v3i(block_coll->loc));
		block_bbox.pos = v3d_sub(block_bbox.pos, entity->center);
		block_bbox.size = v3d_add(block_bbox.size, entity->size);

		if (block_coll->coll_data->coll_type != BLOCK_COLL_NONE)
			movement = entity_collide_bbox(entity, movement, block_bbox);
	}

	entity->ray.pos = v3d_add(entity->ray.pos, movement.dir);

	array_destroy(block_colls, true);
}

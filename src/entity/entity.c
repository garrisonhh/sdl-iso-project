#include <stdio.h>
#include <stdbool.h>
#include "entity.h"
#include "../entity/collision.h"
#include "../lib/vector.h"
#include "../lib/utils.h"
#include "../world.h"
#include "../block/blocks.h"
#include "../block/collision.h"
#include "../camera.h"
#include "../render.h"

void entity_update_directions(entity_t *);

entity_t *entity_create(entity_type_e type, sprite_t *sprite, v3d size) {
	entity_t *entity = malloc(sizeof(entity_t));

	entity->data.type = type;
	// entity state should be set in a wrapper function

	entity->data.sprite = sprite;
	entity->data.anim_state = anim_empty_state();

	entity->data.last_dir = (v3d){0, 1, 0};
	entity->data.dir_xy = DIR_FRONT;
	entity->data.dir_z = DIR_LEVEL;

	entity->data.ray = (ray_t){(v3d){0, 0, 0}, (v3d){0, 0, 0}};
	entity->data.size = size;
	entity->data.center = v3d_scale(entity->data.size, 0.5);

	entity->data.on_ground = false;

	return entity;
}

void entity_destroy(entity_t *entity) {
	switch (entity->data.type) {
	case ENTITY_HUMAN:
		human_destroy(entity->data.state.human);
	default:
		break;
	}

	free(entity);
}

void entity_tick(entity_t *entity, world_t *world, double time) {
	time = MIN(time, 0.1); // super slow physics ticks means broken physics

	// entity state
	switch (entity->data.type) {
	case ENTITY_HUMAN:
		entity_human_tick(entity, time);
		break;
	default:
		break;
	}

	// sprite/animation state
	anim_entity_update_directions(entity);
	anim_tick(entity, entity->data.sprite, &entity->data.anim_state, time);

	// movement
	if (!entity->data.on_ground)
		entity->data.ray.dir.z += GRAVITY * time;

	entity_move_and_collide(entity, world, time);
}

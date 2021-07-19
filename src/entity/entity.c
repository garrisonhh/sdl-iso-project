#include <stdio.h>
#include <stdbool.h>
#include "entity.h"
#include "human.h"
#include <ghh/utils.h>
#include "../entity/collision.h"
#include "../lib/vector.h"
#include "../world.h"
#include "../block/blocks.h"
#include "../block/collision.h"
#include "../camera.h"
#include "../render/render.h"

void entity_destroy(entity_t *entity) {
	switch (entity->data.type) {
	default:
		free(entity);
		break;
	}
}

// TODO rename to entity_populate?
void entity_data_populate(entity_t *entity, entity_type_e type, sprite_t *sprite, v3d size) {
	entity->data.type = type;

	entity->data.sprite = sprite;
	entity->data.anim_state = anim_empty_state();

	entity->data.last_dir = (v3d){0, 1, 0};
	entity->data.dir_xy = DIR_FRONT;
	entity->data.dir_z = DIR_LEVEL;

	entity->data.ray = (ray_t){(v3d){0, 0, 0}, (v3d){0, 0, 0}};
	entity->data.size = size;
	entity->data.center = v3d_scale(entity->data.size, 0.5);

	entity->data.on_ground = false;
}

void entity_tick(entity_t *entity, world_t *world, double time) {
	time = MIN(time, 0.1); // super slow physics ticks means broken physics

	// entity state
	switch (entity->data.type) {
	case ENTITY_HUMAN:
		human_tick((human_t *)entity, time);
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

void entity_add_sprite_packet(array_t *packets, v3i loc, v2i pos,
							  sprite_t *sprite, animation_t *anim_state) {
	render_packet_t *packet = render_sprite_packet_create(loc, pos, sprite);

	packet->sprite.anim = *anim_state;

	array_push(packets, packet);
}

void entity_add_render_info(array_t *packets, entity_t *entity) {
	v3i loc;
	v3d pos;
	v2i screen_pos;

	loc = v3i_from_v3d(entity->data.ray.pos);
	pos = entity->data.ray.pos;
	pos.z -= entity->data.center.z;
	screen_pos = project_v3d(pos);

	render_packet_t *base_packet = render_sprite_packet_create(loc, screen_pos, entity->data.sprite);

	base_packet->sprite.anim = entity->data.anim_state;

	switch (entity->data.type) {
	case ENTITY_BASE:
		array_push(packets, base_packet);
		break;
	case ENTITY_HUMAN:;
		human_t *human = (human_t *)entity;
		sprite_t **sprites = human->hands;

		entity_add_sprite_packet(packets, loc, screen_pos, sprites[0], &human->anim_state);
		array_push(packets, base_packet);
		entity_add_sprite_packet(packets, loc, screen_pos, sprites[1], &human->anim_state);
		break;
	default:
		break;
	}
}

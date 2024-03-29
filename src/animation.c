#include "animation.h"
#include <ghh/utils.h>
#include "entity/entity.h"
#include "camera.h"

// TODO coroutines for these

void anim_human_body(entity_t *, animation_t *);
void anim_human_hands(entity_t *, animation_t *);
void anim_entity_update_directions(entity_t *);

animation_t anim_empty_state() {
	return (animation_t){
		.cell = (v2i){0, 0},
		.state = 0.0,
		.done = false
	};
}

void anim_state_set(animation_t *state, int pose) {
	state->cell.y = pose;
	state->cell.x = 0;
	state->state = 0.0;
	state->done = false;
}

void anim_tick(entity_t *entity, sprite_t *sprite, animation_t *state, double time) {
	switch (sprite->type) {
	default:
		break;
	case SPRITE_HUMAN_BODY:
		anim_human_body(entity, state);
		break;
	case SPRITE_HUMAN_HANDS:
		anim_human_hands(entity, state);
		break;
	}

	if (sprite->anim_lengths[state->cell.y] > 1 && !state->done) {
		state->state += time * ANIMATION_FPS;

		if (state->state > sprite->anim_lengths[state->cell.y]) {
			state->state -= (double)sprite->anim_lengths[state->cell.y];

			switch (sprite->type) {
			default: // animation repeats
				break;
			case SPRITE_HUMAN_TOOL: // animation ends
				state->done = true;
				break;
			}
		}

		state->cell.x = (int)state->state;
	}
}

bool anim_entity_walking(entity_t *entity) {
	return entity->data.dir_z == DIR_LEVEL && !fequals(v3d_magnitude(entity->data.ray.dir), 0.0);
}

void anim_entity_update_directions(entity_t *entity) {
	v3i facing;

	if (!fequals(fabs(entity->data.ray.dir.x) + fabs(entity->data.ray.dir.y), 0)) {
		entity->data.last_dir.x = entity->data.ray.dir.x - entity->data.ray.dir.y;
		entity->data.last_dir.y = entity->data.ray.dir.x + entity->data.ray.dir.y;
	}

	entity->data.last_dir.z = entity->data.ray.dir.z;

	facing = camera_rotated_v3i(polarity_of_v3d(entity->data.last_dir));

	if (facing.z < 0)
		entity->data.dir_z = DIR_DOWN;
	else if (facing.z > 0)
		entity->data.dir_z = DIR_UP;
	else
		entity->data.dir_z = DIR_LEVEL;

	if (facing.y < 0) {
		if (facing.x < 0)
			entity->data.dir_xy = DIR_BACK_LEFT;
		else if (facing.x > 0)
			entity->data.dir_xy = DIR_BACK_RIGHT;
		else
			entity->data.dir_xy = DIR_BACK;
	} else if (facing.y > 0) {
		if (facing.x < 0)
			entity->data.dir_xy = DIR_FRONT_LEFT;
		else if (facing.x > 0)
			entity->data.dir_xy = DIR_FRONT_RIGHT;
		else
			entity->data.dir_xy = DIR_FRONT;
	} else {
		if (facing.x < 0)
			entity->data.dir_xy = DIR_LEFT;
		else if (facing.x > 0)
			entity->data.dir_xy = DIR_RIGHT;
	}
}

void anim_human_body(entity_t *entity, animation_t *state) {
	int pose = 0;

	switch (entity->data.dir_xy) {
	case DIR_FRONT:
		pose = 0;
		break;
	case DIR_BACK:
	case DIR_BACK_LEFT:
	case DIR_BACK_RIGHT:
		pose = 1;
		break;
	case DIR_RIGHT:
	case DIR_FRONT_RIGHT:
		pose = 2;
		break;
	case DIR_LEFT:
	case DIR_FRONT_LEFT:
		pose = 3;
		break;
	}

	if (anim_entity_walking(entity))
		pose += 4;

	if (state->cell.y != pose)
		anim_state_set(state, pose);
}

void anim_human_hands(entity_t *entity, animation_t *state) {
	int pose = 0;

	switch (entity->data.dir_xy) {
	case DIR_FRONT:
		pose = 0;
		break;
	case DIR_BACK:
		pose = 1;
		break;
	case DIR_RIGHT:
	case DIR_FRONT_RIGHT:
	case DIR_BACK_LEFT:
		pose = 2;
		break;
	case DIR_LEFT:
	case DIR_FRONT_LEFT:
	case DIR_BACK_RIGHT:
		pose = 3;
		break;
	}

	if (anim_entity_walking(entity))
		pose += 4;

	if (state->cell.y != pose)
		anim_state_set(state, pose);
}

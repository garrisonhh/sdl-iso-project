#include "textures.h"
#include "entity.h"
#include "utils.h"
#include "animation.h"

void anim_state_set(animation_t *state, int pose) {
	state->cell.y = pose;
	state->cell.x = 0;
	state->state = 0.0;
}

bool anim_entity_walking(entity_t *entity) {
	return entity->dir_z == DIR_LEVEL && !d_close(v3d_magnitude(entity->ray.dir), 0.0);
}

void anim_human_body(entity_t *entity, animation_t *state) {
	int pose;

	switch (entity->dir_xy) {
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
	int pose;

	switch (entity->dir_xy) {
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

void anim_human_tool(entity_t *entity, animation_t *state) {
	// TODO
}

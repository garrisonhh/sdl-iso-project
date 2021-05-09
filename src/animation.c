#include "textures.h"
#include "entity.h"
#include "utils.h"
#include "animation.h"

void anim_state_set(animation_t *state, int anim) {
	state->cell.y = anim;
	state->cell.x = 0;
	state->state = 0.0;
}

bool anim_entity_walking(entity_t *entity) {
	return entity->facing.z == 0 && !d_close(v3d_magnitude(entity->ray.dir), 0.0);
}

void anim_human_body(entity_t *entity, animation_t *state) {
	int anim;
	bool backwards, left, right;

	backwards = entity->facing.y < 0;
	left = entity->facing.x < 0;
	right = entity->facing.x > 0;

	if (anim_entity_walking(entity)) {
		if (backwards)
			anim = 5;
		else if (left)
			anim = 7;
		else if (right)
			anim = 6;
		else
			anim = 4;
	} else { // still
		if (backwards)
			anim = 1;
		else if (left)
			anim = 3;
		else if (right)
			anim = 2;
		else
			anim = 0;
	}

	if (state->cell.y != anim)
		anim_state_set(state, anim);
}

void anim_human_hands(entity_t *entity, animation_t *state) {
	int anim;
	bool backwards, left, right;

	backwards = entity->facing.y < 0;
	left = entity->facing.x < 0;
	right = entity->facing.x > 0;

	if (anim_entity_walking(entity)) {
		if (left) {
			if (backwards)
				anim = 6;
			else
				anim = 7;
		} else if (right) {
			if (backwards)
				anim = 7;
			else
				anim = 6;
		} else if (backwards) {
			anim = 5;
		} else {
			anim = 4;
		}
	} else {
		if (left) {
			if (backwards)
				anim = 2;
			else
				anim = 3;
		} else if (right) {
			if (backwards)
				anim = 3;
			else
				anim = 2;
		} else if (backwards) {
			anim = 1;
		} else {
			anim = 0;
		}
	}

	if (state->cell.y != anim)
		anim_state_set(state, anim);
}

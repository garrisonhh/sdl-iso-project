#include "entity_human.h"
#include "entity.h"
#include "animation.h"

human_t *human_create() {
	human_t *human = malloc(sizeof(human_t));

	human->hands[0] = texture_from_key("harry_back");
	human->hands[1] = texture_from_key("harry_front");

	human->tool = NULL;

	return human;
}

void human_destroy(human_t *human) {
	free(human);
}

entity_t *entity_human_create() {
	v3d size = (v3d){0.4, 0.4, 1.0};

	entity_t *entity = entity_create(ENTITY_HUMAN, texture_from_key("harry_body"), size);

	entity->state.human = human_create();

	return entity;
}

void entity_human_tick(entity_t *entity, double time) {
	human_t *human = entity->state.human;

	if (human->tool == NULL)
		anim_tick(entity, human->hands[0], &human->anim_state, time);
	else
		anim_tick(entity, human->tool->sprites[0], &human->anim_state, time);
}


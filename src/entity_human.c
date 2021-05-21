#include "entity_human.h"
#include "entity.h"
#include "animation.h"
#include "utils.h"

human_t *human_create() {
	human_t *human = malloc(sizeof(human_t));

	human->hands[0] = texture_from_key("harry_back");
	human->hands[1] = texture_from_key("harry_front");

	human->tool = NULL;

	human->tool = malloc(sizeof(tool_t));

	human->tool->sprites[0] = texture_from_key("axe_back");
	human->tool->sprites[1] = texture_from_key("axe_front");

	human->using_tool = false;

	return human;
}

void human_destroy(human_t *human) {
	free(human);
}

entity_t *entity_human_create() {
	v3d size = (v3d){0.7, 0.7, 1.0};

	entity_t *entity = entity_create(ENTITY_HUMAN, texture_from_key("harry_body"), size);

	entity->state.human = human_create();

	return entity;
}

void entity_human_tick(entity_t *entity, double time) {
	human_t *human = entity->state.human;

	if (human->tool == NULL) {
		anim_tick(entity, human->hands[0], &human->anim_state, time);
	} else {
		anim_tick(entity, human->tool->sprites[0], &human->anim_state, time);

		if (human->using_tool && human->anim_state.done)
			human->using_tool = false;
	}
}

void entity_human_use_tool(entity_t *entity) {
	entity->state.human->using_tool = true;
	entity->state.human->anim_state.done = false;
}

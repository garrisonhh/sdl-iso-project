#include "human.h"
#include "entity.h"
#include "../lib/utils.h"

human_t *human_create() {
	v3d size = (v3d){0.7, 0.7, 1.0};
	human_t *human = malloc(sizeof(human_t));

	entity_data_populate((entity_t *)human, ENTITY_HUMAN, sprite_from_key("harry_body"), size);

	human->hands[0] = sprite_from_key("harry_back");
	human->hands[1] = sprite_from_key("harry_front");

	human->anim_state = anim_empty_state();

	return human;
}

void human_tick(human_t *human, double time) {
	anim_tick((entity_t *)human, human->hands[0], &human->anim_state, time);
}

#include "entity_human.h"
#include "entity.h"

human_t *human_create() {
	human_t *human = malloc(sizeof(human_t));

	human->tool = NULL;

	return human;
}

void human_destroy(human_t *human) {
	free(human);
}

entity_t *entity_human_create() {
	texture_t *sprites[3] = {NULL, texture_from_key("harry_body"), NULL};
	texture_t *hands[2] = {texture_from_key("harry_front"), texture_from_key("harry_back")};

	v3d size = (v3d){0.4, 0.4, 1.0};

	entity_t *entity = entity_create(ENTITY_HUMAN, sprites, 3, size);

	memcpy(entity->state.human->hands, hands, sizeof hands);

	return entity;
}

void entity_human_tick(entity_t *entity, double time) {
	human_t *human = entity->state.human;

	// front and back sprites
	texture_t *sides[2];

	if (human->tool == NULL)
		memcpy(sides, human->hands, sizeof human->hands);
	else
		memcpy(sides, human->tool->sprites, sizeof human->tool->sprites);

	entity->sprites[0] = sides[0];
	entity->sprites[2] = sides[1];
}


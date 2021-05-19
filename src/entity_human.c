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
	// TODO
	return NULL;
}

void entity_human_tick(entity_t *entity, double time) {
	// TODO
}

array_t *entity_human_sprites(entity_t *entity) {
	// TODO
	return NULL;
}

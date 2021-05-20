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

entity_t *entity_human_create(v3d pos) {
	texture_t *sprites[3];
	v3d size;

	// TODO human_gen.c
	sprites[0] = texture_from_key("harry_back");
	sprites[1] = texture_from_key("harry_body");
	sprites[2] = texture_from_key("harry_front");

	size = (v3d){0.4, 0.4, 1.0};

	return entity_create(ENTITY_HUMAN, sprites, 3, pos, size);
}

void entity_human_tick(entity_t *entity, double time) {
	// TODO
}

array_t *entity_human_sprites(entity_t *entity) {
	// TODO
	return NULL;
}

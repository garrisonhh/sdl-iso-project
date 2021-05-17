#include "entity.h"
#include "vector.h"
#include "collision.h"
#include "textures.h"

entity_t *player_create() {
	entity_t *player;
	v3d pos, size;

	pos = (v3d){5.0, 5.0, 15.0};
	size = (v3d){0.8, 0.8, 1.0};

	size_t num_sprites = 3;
	texture_t **sprites = malloc(sizeof(sprite_t *) * num_sprites);

	sprites[0] = texture_from_key("harry_back");
	sprites[1] = texture_from_key("harry_body");
	sprites[2] = texture_from_key("harry_front");

	player = entity_create(sprites, num_sprites, pos, size);

	return player;
}


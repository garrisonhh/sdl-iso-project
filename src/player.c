#include "entity.h"
#include "vector.h"
#include "collision.h"

entity_t *player_create() {
	entity_t *new_player = (entity_t *)malloc(sizeof(entity_t));

	new_player->sprite = 0;
	new_player->ray = (ray_t){(v3d){5, 5, 15}, (v3d){0, 0, 0}};
	new_player->size = (v3d){.5, .5, 1};
	
	return new_player;
}


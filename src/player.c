#include "entity.h"
#include "vector.h"
#include "collision.h"

entity_t *player_create() {
	entity_t *player = (entity_t *)malloc(sizeof(entity_t));

	player->sprite = 0;
	player->ray = (ray_t){(v3d){5.0, 5.0, 20.0}, (v3d){0.0, 0.0, 0.0}};
	player->size = (v3d){0.5, 0.5, 1.0};
	player->center = v3d_scale(player->size, 0.5);
	
	return player;
}


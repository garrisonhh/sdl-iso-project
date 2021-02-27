#include <stdlib.h>
#include "entity.h"
#include "vector.h"
#include "collision.h"

void entity_destroy(entity_t *entity) {
	free(entity);
	entity = NULL;
}

void entity_tick(entity_t *entity, int ms) {
	entity->pos = v3d_add(entity->pos, v3d_scale(entity->move, (double)ms / 1000));
}

bbox_t entity_abs_bbox(entity_t *entity) {
	bbox_t box = entity->bbox;
	box.pos = v3d_add(box.pos, entity->pos);
	return box;
}

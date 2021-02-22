#include <stdlib.h>
#include "entity.h"
#include "vector.h"
#include "collision.h"

void destroyEntity(entity_t *entity) {
	free(entity);
	entity = NULL;
}

void tickEntity(entity_t *entity, int ms) {
	entity->pos = dvector3Add(entity->pos, dvector3Scale(entity->move, (double)ms / 1000));
}

bbox_t absoluteBBox(entity_t *entity) {
	bbox_t box = entity->bbox;
	box.offset = dvector3Add(box.offset, entity->pos);
	return box;
}

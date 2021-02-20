#include <stdlib.h>
#include "entity.h"
#include "vector.h"

// TODO movement, collision, time, world-entity and entity-entity interaction

void destroyEntity(entity_t *entity) {
	free(entity);
	entity = NULL;
}

void tickEntity(entity_t *entity, int ms) {
	entity->pos = dvector3Add(entity->pos, dvector3Scale(entity->move, (double)ms / 1000));
}


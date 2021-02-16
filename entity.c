#include <stdlib.h>
#include "entity.h"
#include "vector.h"

// TODO movement, collision, time, world-entity and entity-entity interaction

void destroyEntity(entity_t *entity) {
	free(entity);
	entity = NULL;
}

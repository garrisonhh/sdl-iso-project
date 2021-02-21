#include <stdlib.h>

#include <stdio.h> // DEBUG

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

bbox_t getAbsoluteBBox(entity_t *entity) {
	bbox_t box = entity->bbox;
	
	box.offset = dvector3Add(entity->pos, box.offset);
	
	return box;
}

// resolves collision by moving entity the minimum amount on one of the axes
void resolveEntityCollision(entity_t *entity, bbox_t box) {
	bbox_t entityBox = getAbsoluteBBox(entity);

	if (bboxCollide(entityBox, box)) {
		printf("player pos: %5.2f %5.2f %5.2f\n", entity->pos.x, entity->pos.y, entity->pos.z);
		printf("collided w: %5.0f %5.0f %5.0f\n", box.offset.x, box.offset.y, box.offset.z);

		dvector3 coll = {
			findCollision1d(entityBox.offset.x, entityBox.size.x, entity->move.x, box.offset.x, box.size.x),
			findCollision1d(entityBox.offset.y, entityBox.size.y, entity->move.y, box.offset.y, box.size.y),
			findCollision1d(entityBox.offset.z, entityBox.size.z, entity->move.z, box.offset.z, box.size.z),
		};

		if (coll.z <= coll.y && coll.z <= coll.x)
			entity->pos.z += coll.z;
		else if (coll.y <= coll.x)
			entity->pos.y += coll.y;
		else
			entity->pos.z += coll.x;
	}
}

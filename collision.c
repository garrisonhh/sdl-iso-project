#include <stdbool.h>
#include "collision.h"
#include "vector.h"
#include "entity.h"
#include "world.h"

bool collide1d(double startA, double lenA, double startB, double lenB) {
	return (startA >= startB && startA < startB + lenB)
		|| (startB >= startA && startB < startA + lenA);
}

bool bboxCollide(bbox_t *boxA, bbox_t *boxB) {
	return collide1d(boxA->offset.x, boxA->size.x, boxB->offset.x, boxB->size.x)
		&& collide1d(boxA->offset.y, boxA->size.y, boxB->offset.y, boxB->size.y)
		&& collide1d(boxA->offset.z, boxA->size.z, boxB->offset.z, boxB->size.z);
}

/* 
 * check bboxes of terrain surrounding entity
 * if they collide, kick entity out in the opposite direction of its movement dvector3
 */
void entityCollideWorld(entity_t *entity, world_t *world) {
	// TODO
}

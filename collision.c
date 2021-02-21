#include <stdbool.h>
#include "collision.h"
#include "vector.h"
#include "entity.h"
#include "world.h"

bool collide1d(double startA, double lenA, double startB, double lenB) {
	return (startA > startB && startA < startB + lenB)
		|| (startB > startA && startB < startA + lenA);
}

bool bboxCollide(bbox_t boxA, bbox_t boxB) {
	return collide1d(boxA.offset.x, boxA.size.x, boxB.offset.x, boxB.size.x)
		&& collide1d(boxA.offset.y, boxA.size.y, boxB.offset.y, boxB.size.y)
		&& collide1d(boxA.offset.z, boxA.size.z, boxB.offset.z, boxB.size.z);
}

// returns offset for startE in order to resolve collision
// assumes that entity has collided with box by traveling in direction of velocity
double findCollision1d(double startE, double lenE, double velE, double startB, double lenB) {
	if (velE >= 0)
		return startB - (startE + lenE);
	return (startB + lenB) - startE;
}


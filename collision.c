#include <stdbool.h>
#include "collision.h"
#include "vector.h"

bool collide1d(double startA, double lenA, double startB, double lenB) {
	return (startA >= startB && startA < startB + lenB)
		|| (startB >= startA && startB < startA + lenA);
}

bool bboxCollide(bbox_t *boxA, bbox_t *boxB) {
	return collide1d(boxA->offset.x, boxA->size.x, boxB->offset.x, boxB->size.x)
		&& collide1d(boxA->offset.y, boxA->size.y, boxB->offset.y, boxB->size.y)
		&& collide1d(boxA->offset.z, boxA->size.z, boxB->offset.z, boxB->size.z);
}

#ifndef COLLISION_H
#define COLLISION_H

#include <stdbool.h>
#include "vector.h"
// #include "entity.h"

// bbox's are relative to entities
typedef struct {
	dvector3 offset;
	dvector3 size;
} bbox_t;

<<<<<<< HEAD
bool bboxCollide(bbox_t, bbox_t);

// used by resolveCollision in entity.c
// TODO figure out the header issues here (circular includes)
double findCollision1d(double, double, double, double, double);
=======
void printfBBox(bbox_t);
dvector3 collideResolveMultiple(bbox_t eBox, bbox_t *boxArr, int lenArr);
>>>>>>> 0ddeedebff5cc2b35e210e4113f6ff331dee8759

#endif

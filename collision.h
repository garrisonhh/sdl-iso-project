#ifndef COLLISION_H
#define COLLISION_H

#include <stdbool.h>
#include "vector.h"

// bbox's are relative to entities
typedef struct {
	dvector3 offset;
	dvector3 size;
} bbox_t;

#endif

#ifndef COLLISION_H
#define COLLISION_H

#include <stdbool.h>
#include "vector.h"

// bbox's are relative to entities
typedef struct {
	v3d pos;
	v3d size;
} bbox_t;

#endif

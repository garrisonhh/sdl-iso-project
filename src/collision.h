#ifndef COLLISION_H
#define COLLISION_H

#include <stdbool.h>
#include "vector.h"

// bbox's are relative to entities
typedef struct {
	v3d offset;
	v3d size;
} bbox_t;

v3d collideResolveMultiple(bbox_t eBox, bbox_t *boxArr, int lenArr);

#endif

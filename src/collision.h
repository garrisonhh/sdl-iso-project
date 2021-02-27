#ifndef COLLISION_H
#define COLLISION_H

#include <stdbool.h>
#include "vector.h"

// bbox's are relative to entities
typedef struct {
	v3d pos;
	v3d size;
} bbox_t;

typedef struct {
	v3d pos;
	v3d dir;
} ray_t;

void ray_bbox_intersection(v3d *, v3d *, int *, ray_t, bbox_t);
void bbox_print(bbox_t);
void sort_bboxes_by_vector_polarity(bbox_t *, int, v3d);

#endif

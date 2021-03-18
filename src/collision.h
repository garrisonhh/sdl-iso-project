#ifndef COLLISION_H
#define COLLISION_H

#include "vector.h"
#include "list.h"

extern const v3d BLOCK_SIZE;

struct bbox_t {
	v3d pos;
	v3d size;
};
typedef struct bbox_t bbox_t;

struct ray_t {
	v3d pos;
	v3d dir;
};
typedef struct ray_t ray_t;

void ray_bbox_intersection(v3d *, v3d *, int *, ray_t, bbox_t);
void sort_bboxes_by_vector_polarity(list_t *, v3d);

#endif

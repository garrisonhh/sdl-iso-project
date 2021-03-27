#ifndef COLLISION_H
#define COLLISION_H

#include <stdbool.h>
#include "vector.h"
#include "list.h"

// extern const v3d BLOCK_SIZE;

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

struct sphere_t {
	v3d pos;
	double radius;
};
typedef struct sphere_t sphere_t;

// void sort_bboxes_by_vector_polarity(list_t *, v3d);
int ray_bbox_intersection(ray_t, bbox_t, v3d *, v3d *);
bool line_sphere_intersection(ray_t, sphere_t, v3d *);
bool ray_sphere_intersection(ray_t, sphere_t, v3d *);
bool ray_plane_intersection(ray_t, ray_t, v3d *, v3d *);

#endif

#ifndef COLLISION_H
#define COLLISION_H

#include <stdbool.h>
#include "lib/vector.h"

typedef struct bbox {
	v3d pos;
	v3d size;
} bbox_t;

typedef struct ray {
	v3d pos;
	v3d dir;
} ray_t;

typedef struct sphere {
	v3d pos;
	double radius;
} sphere_t;

bool inside_bbox(bbox_t, v3d);

int ray_intersects_bbox(ray_t, bbox_t, v3d *intersect, v3d *resolved_dir);
// plane represented as its normal
bool ray_intersects_plane(ray_t ray, ray_t plane, v3d *intersect, v3d *resolved_dir, bool *behind);
bool ray_intersects_sphere(ray_t, sphere_t, v3d *intersect);

#endif

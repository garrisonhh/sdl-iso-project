#ifndef COLLISION_H
#define COLLISION_H

#include <stdbool.h>
#include "vector.h"
#include "list.h"

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

bool inside_bbox(bbox_t, v3d);
int ray_intersects_bbox(ray_t, bbox_t, v3d *, v3d *);
// bool line_intersects_sphere(ray_t, sphere_t, v3d *);
// bool ray_intersects_sphere(ray_t, sphere_t, v3d *);
bool ray_intersects_plane(ray_t, ray_t, v3d *, v3d *, bool *);

#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "list.h"
#include "collision.h"
#include "vector.h"
#include "utils.h"

const v3d BLOCK_SIZE = {1.0, 1.0, 1.0};
v3i BBOX_SORT_POLARITY = {1, 1, 1};

bool collides(double a, double b, double x) {
	return (a < x) && (x < b);
}

bool collide1d(double start_a, double len_a, double start_b, double len_b) {
	return collides(start_b, start_b + len_b, start_a)
		|| collides(start_b, start_b + len_b, start_a + len_a)
		|| (d_close(start_a, start_b) && d_close(len_a, len_b));
}

bool bbox_bbox_collide(bbox_t a, bbox_t b) {
	for (int i = 0; i < 3; i++) {
		if (!collide1d(v3d_get(&a.pos, i), v3d_get(&a.size, i),
					   v3d_get(&b.pos, i), v3d_get(&b.size, i))) {
			return false;
		}
	}
	return true;
}

bool inside_bbox(bbox_t box, v3d point) {
	double dim_start, dim_end, dim_point;

	for (int i = 0; i < 3; i++) {
		dim_start = v3d_get(&box.pos, i);
		dim_end = dim_start + v3d_get(&box.size, i);
		dim_point = v3d_get(&point, i);

		if (!(collides(dim_start, dim_end, dim_point)
		   || d_close(dim_start, dim_point) || d_close(dim_end, dim_point)))
			return false;
	}

	return true;
}

v3d bbox_center(bbox_t box) {
	return v3d_add(box.pos, v3d_scale(box.size, .5));
}

int bbox_compare(const void *a, const void *b) {
	v3d center_a = bbox_center(**(bbox_t **)a);
	v3d center_b = bbox_center(**(bbox_t **)b);
	int i;
	double comparison, polarity;

	// TODO simplify the if statement, should be possible
	for (i = 2; i >= 0; i--) {
		comparison = v3d_get(&center_b, i) - v3d_get(&center_a, i);
		polarity = v3i_get(&BBOX_SORT_POLARITY, i) > 0;

		if (d_close(comparison, 0))
			continue;
		else if (comparison > 0)
			return polarity > 0 ? -1 : 1;
		else if (comparison < 0)
			return polarity > 0 ? 1 : -1;
	}

	return 1;
}

void sort_bboxes_by_vector_polarity(list_t *boxes, v3d v) {
	BBOX_SORT_POLARITY = polarity_of_v3d(v);
	qsort(boxes->items, boxes->size, sizeof(void *), bbox_compare);
}

bbox_t ray_to_bbox(ray_t ray) {
	bbox_t box;
	box.pos = ray.pos;
	box.size = ray.dir;
	return box;
}

/*
returns axis of intersection (-1 for no intersection)
intersection point into intersection
for collision resolution purposes, determines best modification of ray and outputs
into resolved_dir
*/
int ray_bbox_intersection(ray_t ray, bbox_t box, v3d *intersection, v3d *resolved_dir) {
	double plane, plane_vel;
	double dim_start, dim_len;
	double axis_vel, axis_res;
	int i, j;
	v3d isect;
	bool collide;

	for (i = 2; i >= 0; i--) {
		// find near plane of face on this axis 
		plane_vel = v3d_get(&ray.dir, i);
		plane = v3d_get(&box.pos, i);
	
		if (v3d_get(&ray.dir, i) < 0)
			plane += v3d_get(&box.size, i);

		if (!d_close(plane_vel, 0)) {
			// ray is not parallel, safe to find line-box intersection
			collide = true;
			for (j = 0; j < 3; j++) {
				if (j == i)
					continue;

				// axis_res = collision with plane along slope b/t axis j and plane axis
				axis_vel = v3d_get(&ray.dir, j) / plane_vel;
				axis_res = (axis_vel * plane) + (v3d_get(&ray.pos, j) - (axis_vel * v3d_get(&ray.pos, i)));

				// check collision is in bounds of the box face on this plane
				dim_start = v3d_get(&box.pos, j);
				dim_len = v3d_get(&box.size, j);

				if ((dim_start <= axis_res && axis_res <= dim_start + dim_len)
				 || d_close(dim_start, axis_res) || d_close(dim_start + dim_len, axis_res)) {
					v3d_set(&isect, j, axis_res);
				} else {
					collide = false;
					break;
				}
			}
			
			if (collide) {
				v3d_set(&isect, i, plane);

				// check collision in bounds of ray
				if (bbox_bbox_collide(ray_to_bbox(ray), box)) {
					if (intersection != NULL)
						*intersection = isect;
					if (resolved_dir != NULL) {
						*resolved_dir = ray.dir;
						v3d_set(resolved_dir, i, plane - v3d_get(&ray.pos, i)); // pixel fucking perfect :)
					}
					return i;
				}
			}
		}
	}

	// no collision on any of the box faces
	return -1;
}

// TODO check if collision point in range of ray
bool line_sphere_intersection(ray_t ray, sphere_t sphere, v3d *intersection) {
	if (d_close(v3d_magnitude(ray.dir), 0.0)) {
		printf("attempted ray sphere intersection with ray of 0 magnitude.\n");
		exit(1);
	}

	double a, b, c;
	double b4ac_term, sqrt_term, b2a_term;
	double t1, t2;

	a = v3d_dot(ray.dir, ray.dir);
	b = 2.0 * v3d_dot(ray.dir, v3d_sub(ray.pos, sphere.pos));
	c = v3d_dot(sphere.pos, sphere.pos) + v3d_dot(ray.pos, ray.pos)
		- (2.0 * v3d_dot(sphere.pos, ray.pos)) - (sphere.radius * sphere.radius);

	// now solve quadratic equation
	b4ac_term = b * b - (4.0 * a * c);

	if (b4ac_term < 0.0) {
		return false;
	} else if (intersection != NULL) {
		sqrt_term = sqrt(b4ac_term) / (2.0 * a);
		b2a_term = -b / (2.0 * a);

		t1 = b2a_term + sqrt_term;
		t2 = b2a_term - sqrt_term;
		
		*intersection = v3d_add(ray.pos, v3d_scale(ray.dir, MIN(t1, t2)));
	}

	return true;
}

bool ray_sphere_intersection(ray_t ray, sphere_t sphere, v3d *intersection) {
	v3d line_intersect;

	if (line_sphere_intersection(ray, sphere, &line_intersect)
	 && inside_bbox(ray_to_bbox(ray), line_intersect)) {
		if (intersection != NULL)
			*intersection = line_intersect;
		return true;
	}

	return false;
}

bool ray_plane_intersection(ray_t ray, ray_t plane, v3d *intersection, v3d *resolved_dir) {
	double zx, sin_zx, cos_zx;
	double zpy, sin_zpy, cos_zpy;
	double zp;
	ray_t rotated;

	// angle from z to x
	zx = atan2(plane.dir.x, plane.dir.z);
	sin_zx = sin(zx);
	cos_zx = cos(zx);

	// angle from z prime to y
	zpy = atan2(plane.dir.y, plane.dir.z * cos_zx + plane.dir.x * sin_zx);
	sin_zpy = sin(zpy);
	cos_zpy = cos(zpy);

	// find rotated ray
	ray.pos = v3d_sub(ray.pos, plane.pos);

	rotated.pos.x = ray.pos.x * cos_zx - ray.pos.z * sin_zx;
	zp = ray.pos.z * cos_zx + ray.pos.x * sin_zx;
	rotated.pos.y = ray.pos.y * cos_zpy - zp * sin_zpy;
	rotated.pos.z = zp * cos_zpy + ray.pos.y * sin_zpy;

	rotated.dir.x = ray.dir.x * cos_zx - ray.dir.z * sin_zx;
	zp = ray.dir.z * cos_zx + ray.dir.x * sin_zx;
	rotated.dir.y = ray.dir.y * cos_zpy - zp * sin_zpy;
	rotated.dir.z = zp * cos_zpy + ray.dir.y * sin_zpy;

	printf("rotated pos: %.4f %.4f %.4f\n", rotated.pos.x, rotated.pos.y, rotated.pos.z);
	printf("rotated dir: %.4f %.4f %.4f\n", rotated.dir.x, rotated.dir.y, rotated.dir.z);

	// check for collision (whether rotated ray points into negative z)
	if (rotated.pos.z + rotated.dir.z >= 0) {
		return false;
	} else {
		double t_intersect = -rotated.pos.z / rotated.dir.z;

		printf("t_intersect: %.4f\n", t_intersect);

		if (t_intersect > 1) // collision outside of ray bounds
			return false;

		// find and put values in pointers if requested
		if (intersection != NULL)
			*intersection = v3d_add(ray.pos, v3d_scale(ray.dir, t_intersect));
		if (resolved_dir != NULL) {
			double t_resolve = -(rotated.pos.z + rotated.dir.z) / v3d_magnitude(plane.dir);
			*resolved_dir = v3d_add(ray.dir, v3d_scale(plane.dir, t_resolve));
		}

		return true;
	}
}


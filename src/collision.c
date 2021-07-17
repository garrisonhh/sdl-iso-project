#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "collision.h"
#include "lib/vector.h"
#include <ghh/utils.h>

bool collides(double a, double b, double x) {
	return (a < x) && (x < b);
}

bool collide1d(double start_a, double len_a, double start_b, double len_b) {
	return collides(start_b, start_b + len_b, start_a)
		|| collides(start_b, start_b + len_b, start_a + len_a)
		|| (fequals(start_a, start_b) && fequals(len_a, len_b));
}

bool bbox_bbox_collide(bbox_t a, bbox_t b) {
	for (int i = 0; i < 3; i++) {
		if (!collide1d(v3d_IDX(a.pos, i), v3d_IDX(a.size, i),
					   v3d_IDX(b.pos, i), v3d_IDX(b.size, i))) {
			return false;
		}
	}
	return true;
}

bool inside_bbox(bbox_t box, v3d point) {
	double dim_start, dim_end, dim_point;

	for (int i = 0; i < 3; i++) {
		dim_start = v3d_IDX(box.pos, i);
		dim_end = dim_start + v3d_IDX(box.size, i);
		dim_point = v3d_IDX(point, i);

		if (!(collides(dim_start, dim_end, dim_point)
		   || fequals(dim_start, dim_point) || fequals(dim_end, dim_point)))
			return false;
	}

	return true;
}

bbox_t ray_to_bbox(ray_t ray) {
	bbox_t box;

	box.pos = ray.pos;
	box.size = ray.dir;

	for (int i = 0; i < 3; ++i) {
		if (v3d_IDX(box.size, i) < 0.0) {
			v3d_IDX(box.pos, i) += v3d_IDX(box.size, i);
			v3d_IDX(box.size, i) = -v3d_IDX(box.size, i);
		}
	}

	return box;
}

/*
 * returns axis of intersection (-1 for no intersection)
 * intersection point -> intersection
 * modified ray direction -> resolved_dir
 */
int ray_intersects_bbox(ray_t ray, bbox_t box, v3d *intersection, v3d *resolved_dir) {
	double plane, plane_vel;
	double dim_start, dim_len;
	double axis_vel, axis_res;
	int i, j;
	v3d isect;
	bool collide;

	for (i = 2; i >= 0; i--) {
		// find near plane of face on this axis
		plane_vel = v3d_IDX(ray.dir, i);
		plane = v3d_IDX(box.pos, i);

		if (v3d_IDX(ray.dir, i) < 0)
			plane += v3d_IDX(box.size, i);

		if (!fequals(plane_vel, 0)) {
			// ray is not parallel, safe to find line-box intersection
			collide = true;
			for (j = 0; j < 3; j++) {
				if (j == i)
					continue;

				// axis_res = collision with plane along slope b/t axis j and plane axis
				axis_vel = v3d_IDX(ray.dir, j) / plane_vel;
				axis_res = (axis_vel * plane) + (v3d_IDX(ray.pos, j) - (axis_vel * v3d_IDX(ray.pos, i)));

				// check collision is in bounds of the box face on this plane
				dim_start = v3d_IDX(box.pos, j);
				dim_len = v3d_IDX(box.size, j);

				if ((dim_start <= axis_res && axis_res <= dim_start + dim_len)
				 || fequals(dim_start, axis_res) || fequals(dim_start + dim_len, axis_res)) {
					v3d_IDX(isect, j) = axis_res;
				} else {
					collide = false;
					break;
				}
			}

			if (collide) {
				v3d_IDX(isect, i) = plane;

				// check collision in bounds of ray
				if (bbox_bbox_collide(ray_to_bbox(ray), box)) {
					if (intersection != NULL)
						*intersection = isect;
					if (resolved_dir != NULL) {
						v3d resolved = ray.dir;
						v3d_IDX(resolved, i) = plane - v3d_IDX(ray.pos, i); // pixel fucking perfect :)

						*resolved_dir = resolved;
					}
					return i;
				}
			}
		}
	}

	// no collision on any of the box faces
	return -1;
}

/*
 * returns whether ray intersects plane
 * intersection point -> intersection
 * new ray dir as if pushed out of plane -> resolved_dir
 * ray points behind plane (regardless of intersection) -> behind
 */
bool ray_intersects_plane(ray_t ray, ray_t plane, v3d *intersection, v3d *resolved_dir, bool *behind) {
	double zx, sin_zx, cos_zx;
	double zpy, sin_zpy, cos_zpy;
	double zp;
	double z_sum;
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

	// check for collision (whether rotated ray points into negative z)
	if ((z_sum = rotated.pos.z + rotated.dir.z) >= 0 || fequals(z_sum, 0.0)) {
		if (behind != NULL)
			*behind = false;

		return false;
	} else {
		if (behind != NULL)
			*behind = true;

		double t_intersect = rotated.pos.z / rotated.dir.z;

		if (t_intersect >= 1 || fequals(t_intersect, 1.0)) // collision outside of ray bounds
			return false;

		// find and put values in pointers if requested
		if (intersection != NULL)
			*intersection = v3d_add(v3d_add(plane.pos, ray.pos), v3d_scale(ray.dir, t_intersect));
		if (resolved_dir != NULL) {
			double t_resolve = -z_sum / v3d_magnitude(plane.dir);
			*resolved_dir = v3d_add(ray.dir, v3d_scale(plane.dir, t_resolve));
		}

		return true;
	}
}

// does not restrict to ray bounds
bool line_intersects_sphere(ray_t ray, sphere_t sphere, v3d *intersection) {
	if (fequals(v3d_magnitude(ray.dir), 0.0)) {
		printf("attempted ray sphere intersection with ray of 0 magnitude.\n");
		exit(1);
	}

	double a, b, c;
	double b4ac_term;
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
		double sqrt_term, b2a_term;

		sqrt_term = sqrt(b4ac_term) / (2.0 * a);
		b2a_term = -b / (2.0 * a);

		t1 = b2a_term + sqrt_term;
		t2 = b2a_term - sqrt_term;

		*intersection = v3d_add(ray.pos, v3d_scale(ray.dir, MIN(t1, t2)));
	}

	return true;
}

// restricts to ray bounds
bool ray_intersects_sphere(ray_t ray, sphere_t sphere, v3d *intersection) {
	v3d line_intersect;

	if (line_intersects_sphere(ray, sphere, &line_intersect)
	 && (inside_bbox(ray_to_bbox(ray), line_intersect)
	  || v3d_dist(ray.pos, line_intersect) < sphere.radius
	  || v3d_dist(v3d_add(ray.pos, ray.dir), line_intersect) < sphere.radius)) {
		if (intersection != NULL)
			*intersection = line_intersect;
		return true;
	}

	return false;
}

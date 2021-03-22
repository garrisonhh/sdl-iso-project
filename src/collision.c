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

bbox_t ray_to_bbox(ray_t ray) {
	bbox_t box;
	box.pos = ray.pos;
	box.size = ray.dir;
	return box;
}

// returns axis of intersection (-1 for no intersection)
// outputs values into intersection and resolved_dir
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

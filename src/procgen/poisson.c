#include <stdio.h> // TODO remove
#include <stdlib.h>
#include <math.h>
#include "poisson.h"
#include "../lib/utils.h"

struct poisson_grid2_t {
	v2i **points;
	int w, h;
	int total_w, total_h;
	double cell_side, r;
};
typedef struct poisson_grid2_t poisson_grid2_t;

void poisson_grid2_populate(poisson_grid2_t *grid, double r, int w, int h) {
	grid->r = r;
	grid->cell_side = grid->r / SQRT2;

	grid->w = (int)((double)w / grid->cell_side) + 1;
	grid->h = (int)((double)h / grid->cell_side) + 1;
	grid->total_w = w;
	grid->total_h = h;

	grid->points = malloc(sizeof(v2i *) * grid->w * grid->h);

	for (int i = 0; i < grid->w * grid->h; ++i)
		grid->points[i] = NULL;
}

// returns success
bool poisson_grid2_place(poisson_grid2_t *grid, v2i *pos) {
	if (!(INRANGE(pos->x, 0, grid->total_w) && INRANGE(pos->y, 0, grid->total_h)))
		return false;

	int x, y;
	int ax, ay;
	v2i *other;

	x = (double)pos->x / grid->cell_side;
	y = (double)pos->y / grid->cell_side;

	for (ay = MAX(y - 2, 0); ay <= MIN(y + 2, grid->h - 1); ++ay) {
		for (ax = MAX(x - 2, 0); ax <= MIN(x + 2, grid->w - 1); ++ax) {
			if ((other = grid->points[ay * grid->w + ax]) != NULL
			 && hypot(other->x - pos->x, other->y - pos->y) < grid->r)
				return false;
		}
	}

	grid->points[y * grid->w + x] = pos;

	return true;
}

array_t *poisson_samples2(int width, int height, double r, int k) {
	int i;
	double angle, dist;
	poisson_grid2_t grid;
	v2i *pos, *other;
	array_t *active, *samples;
	bool placed;

	poisson_grid2_populate(&grid, r, width, height);
	active = array_create(0);

	pos = malloc(sizeof(v2i));
	pos->x = randf() * (double)width;
	pos->y = randf() * (double)height;

	poisson_grid2_place(&grid, pos);
	array_push(active, pos);

	while (active->size) {
		if (active->size > 100000) {
			printf("exceeded limit.\n");
			exit(0);
		}

		pos = array_pop(active);
		placed = false;

		for (i = 0; i < k; ++i) {
			angle = randf() * 2.0 * M_PI;
			dist = r + randf() * r;

			other = malloc(sizeof(v2i));
			*other = *pos;

			other->x += cos(angle) * dist;
			other->y += sin(angle) * dist;
			
			if (poisson_grid2_place(&grid, other)) {
				array_push(active, other);
				placed = true;

				break;
			}
		}

		if (placed)
			array_push(active, pos);
	}

	array_destroy(active, false);

	samples = array_create(0);

	for (i = 0; i < grid.w * grid.h; ++i)
		if (grid.points[i] != NULL)
			array_push(samples, grid.points[i]);

	free(grid.points);

	return samples;
}

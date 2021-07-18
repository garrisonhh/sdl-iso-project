#include <stdio.h> // TODO remove
#include <stdlib.h>
#include <math.h>
#include <ghh/utils.h>
#include "poisson.h"

typedef struct poisson2_grid {
	v2i **points;
	int w, h;
	int total_w, total_h;
	double cell_side, r;
} poisson2_grid_t;

typedef struct poisson2_sample {
	v2i *pos;
	double value;
} poisson2_sample_t;

void poisson2_grid_populate(poisson2_grid_t *grid, double r, int w, int h) {
	grid->r = r;
	grid->cell_side = grid->r / M_SQRT2;

	grid->w = (int)((double)w / grid->cell_side) + 1;
	grid->h = (int)((double)h / grid->cell_side) + 1;
	grid->total_w = w;
	grid->total_h = h;

	grid->points = malloc(sizeof(v2i *) * grid->w * grid->h);

	for (int i = 0; i < grid->w * grid->h; ++i)
		grid->points[i] = NULL;
}

// returns success
bool poisson2_grid_place(poisson2_grid_t *grid, v2i *pos) {
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

array_t *poisson2_samples(int width, int height, double r, int k) {
	int i;
	double angle, dist;
	poisson2_grid_t grid;
	v2i *pos, *other;
	array_t *active, *samples;
	bool placed;

	// create grid
	poisson2_grid_populate(&grid, r, width, height);
	active = array_create(0);

	// place starting point
	pos = malloc(sizeof(v2i));
	pos->x = randf() * (double)width;
	pos->y = randf() * (double)height;

	poisson2_grid_place(&grid, pos);
	array_push(active, pos);

	while (array_size(active)) {
		// pop last point
		pos = array_pop(active);
		placed = false;

		// generate k samples around points
		for (i = 0; i < k; ++i) {
			angle = randf() * 2.0 * M_PI;
			dist = r + randf() * r;

			other = malloc(sizeof(v2i));
			*other = *pos;

			other->x += cos(angle) * dist;
			other->y += sin(angle) * dist;

			// if a sample can be placed, push it into active
			if (poisson2_grid_place(&grid, other)) {
				array_push(active, other);
				placed = true;

				break;
			}
		}

		// if sample was placed, push original point back into active
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

int poisson2_sample_compare(const void *a, const void *b) {
	double v = ((**(poisson2_sample_t **)a).value - (**(poisson2_sample_t **)b).value);

	if (fequals(v, 0))
		return 0;
	return (v > 0 ? 1 : -1);
}

void poisson2_prune_sort_samples(array_t *samples, noise2_t *noise, bool reverse) {
	size_t i;
	array_t *pruned = array_create(array_size(samples));
	poisson2_sample_t *sample;

	// create sample packets and sort them
	for (i = 0; i < array_size(samples); ++i) {
		sample = malloc(sizeof(poisson2_sample_t));

		sample->pos = array_get(samples, i);
		sample->value = noise2_at(noise, sample->pos->x, sample->pos->y);

		array_push(pruned, sample);
	}

	array_qsort(pruned, poisson2_sample_compare);

	// reorganize sample positions back into original array
	array_clear(samples, false);

	if (reverse) {
		while (array_size(pruned)) {
			sample = array_pop(pruned);
			array_push(samples, sample->pos);
		}
	} else {
		for (i = 0; i < array_size(pruned); ++i) {
			sample = array_get(pruned, i);
			array_push(samples, sample->pos);
		}
	}

	array_destroy(pruned, true);
}

void poisson2_prune_worst(array_t *samples, noise2_t *noise, double percentage) {
	int n_remove = array_size(samples) * percentage;

	poisson2_prune_sort_samples(samples, noise, true);

	for (int i = 0; i < n_remove; ++i)
		free(array_pop(samples));
}

void poisson2_prune_best(array_t *samples, noise2_t *noise, double percentage) {
	int n_remove = array_size(samples) * percentage;

	poisson2_prune_sort_samples(samples, noise, false);

	for (int i = 0; i < n_remove; ++i)
		free(array_pop(samples));
}

void poisson2_prune_below(array_t *samples, noise2_t *noise, double value) {
	v2i *pos;
	poisson2_prune_sort_samples(samples, noise, true);

	while (array_size(samples)) {
		pos = array_peek(samples);

		if (noise2_at(noise, pos->x, pos->y) >= value)
			break;
		else
			free(array_pop(samples));
	}
}

void poisson2_prune_above(array_t *samples, noise2_t *noise, double value) {
	v2i *pos;
	poisson2_prune_sort_samples(samples, noise, false);

	while (array_size(samples)) {
		pos = array_peek(samples);

		if (noise2_at(noise, pos->x, pos->y) <= value)
			break;
		else
			free(array_pop(samples));
	}
}

void poisson2_prune_linear(array_t *samples, noise2_t *noise, double percentage) {
	int n_remove = array_size(samples) * percentage;
	int index;

	poisson2_prune_sort_samples(samples, noise, false);

	for (int i = 0; i < n_remove; ++i) {
		index = randf() * randf() * array_size(samples);

		array_del(samples, index);
	}
}

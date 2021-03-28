#include <SDL2/SDL.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include "vector.h"
#include "render.h"
#include "render_primitives.h"

// only applicable where deltax > deltay and x and y are positive
// so the main bresenham function handles the paperwork and this does the fun stuff
// **assumes points is already the right size**
void bresenham_down_right(v2i delta, v2i *points) {
	v2i pos = (v2i){0, 0};
	double delta_err = fabs((double)delta.y / (double)delta.x);
	double error = 0.0;

	for (int i = 0; i < delta.x; i++) {
		points[i] = pos;
		pos.x++;

		if ((error += delta_err) >= 0.5) {
			pos.y++;
			error--;
		}
	}
}

// **need to free() points afterwards**
void bresenham(v2i a, v2i b, v2i **points, size_t *num_points) {
	if (a.x == b.x && a.y == b.y) {
		*num_points = 0;
		return;
	}

	v2i delta;
	bool flipx, flipy, flipd;
	v2i current;

	delta = v2i_sub(b, a);

	flipx = delta.x < 0;
	flipy = delta.y < 0;
	flipd = abs(delta.y) > abs(delta.x);

	if (flipx)
		delta.x = -delta.x;
	if (flipy)
		delta.y = -delta.y;
	if (flipd)
		delta = (v2i){delta.y, delta.x};

	*num_points = delta.x;
	*points = (v2i *)calloc(*num_points, sizeof(v2i));

	bresenham_down_right(delta, *points);

	for (size_t i = 0; i < *num_points; i++) {
		current = (*points)[i];

		if (flipx)
			current.x = -current.x;
		if (flipy)
			current.y = -current.y;
		if (flipd)
			current = (v2i){current.y, current.x};

		(*points)[i] = v2i_add(a, current);
	}
}

void render_iso_circle(circle_t circle) {
	int i, ix;
	float y = 0.0, y_step = 2 / (float)circle.radius;

	SDL_RenderDrawLine(renderer, circle.loc.x + circle.radius, circle.loc.y,
								 circle.loc.x - circle.radius, circle.loc.y);

	for (i = 1; i <= circle.radius >> 1; i++) {
		ix = (int)(sqrt(1 - (y * y)) * circle.radius);
		y += y_step;

		SDL_RenderDrawLine(renderer, circle.loc.x + ix, circle.loc.y + i,
									 circle.loc.x - ix, circle.loc.y + i);
		SDL_RenderDrawLine(renderer, circle.loc.x + ix, circle.loc.y - i,
									 circle.loc.x - ix, circle.loc.y - i);
	}
}

// only works for convex polygons
// TODO debug
void render_filled_poly(v2i *points, size_t num_points) {
	int min_y, max_y, delta_y;
	int *x_pos, x1, x2;
	size_t i, j;
	v2i *line;
	size_t line_size;

	for (i = 0; i < num_points; i++) {
		if (points[i].y > max_y)
			max_y = points[i].y;
		if (points[i].y < min_y)
			min_y = points[i].y;
	}

	delta_y = max_y - min_y + 1;
	list_t *lines[delta_y];

	for (i = 0; i < delta_y; i++)
		lines[i] = list_create();

	for (i = 0; i < num_points; i++) {
		bresenham(points[i], points[(i + 1) % num_points], &line, &line_size);

		for (j = 0; j < line_size; j++) {
			x_pos = (int *)malloc(sizeof(int));
			*x_pos = line[j].x;
			list_add(lines[line[j].y - min_y], x_pos);
		}

		free(line);
	}

	for (i = 0; i < delta_y; i++) {
		if (lines[i]->size == 1) {
			SDL_RenderDrawPoint(renderer, *(int *)lines[i]->items[0], min_y + i);
		} else if (lines[i]->size == 2) {
			x1 = *(int *)lines[i]->items[0];
			x2 = *(int *)lines[i]->items[1];
			SDL_RenderDrawLine(renderer, x1, min_y + i, x2, min_y + i);
		}
	}

	for (i = 0; i < delta_y; i++)
		list_deep_destroy(lines[i]);
}

void render_filled_box(box_t box) {
	// TODO
}

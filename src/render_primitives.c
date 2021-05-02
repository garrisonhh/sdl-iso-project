#include <SDL2/SDL.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include "vector.h"
#include "render.h"
#include "render_primitives.h"
#include "utils.h"

// only applicable where deltax > deltay and x and y are positive
// so the main bresenham function handles the paperwork and this does the fun stuff
// **assumes points is already the right size**
void bresenham_down_right(v2i delta, v2i *points) {
	v2i pos = (v2i){0, 0};
	double delta_err = fabs((double)delta.y / (double)delta.x);
	double error = 0.0;

	for (int i = 0; i <= delta.x; i++) {
		points[i] = pos;
		pos.x++;

		if ((error += delta_err) >= 0.5) {
			pos.y++;
			error--;
		}
	}
}

// **need to free() points afterwards**
v2i *bresenham(v2i a, v2i b, size_t *num_points) {
	if (a.x == b.x && a.y == b.y) {
		*num_points = 0;
		return NULL;
	}

	v2i delta, current;
	v2i *points;
	bool flipx, flipy, flipd;

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

	*num_points = delta.x + 1;
	points = calloc(*num_points, sizeof(v2i));

	bresenham_down_right(delta, points);

	for (size_t i = 0; i < *num_points; i++) {
		current = points[i];

		if (flipx)
			current.x = -current.x;
		if (flipy)
			current.y = -current.y;
		if (flipd)
			current = (v2i){current.y, current.x};

		points[i] = v2i_add(a, current);
	}

	return points;
}

// only works for convex polygons
void render_filled_poly(v2i *points, size_t num_points) {
	if (num_points < 3) {
		printf("attempted to render polygon with too few points.\n");
		exit(1);
	}

	int min_y, max_y, range_y;
	int py, px;
	size_t i, j;
	v2i *line;
	size_t line_size;

	min_y = max_y = points[0].y;

	for (i = 1; i < num_points; i++) {
		if (points[i].y > max_y)
			max_y = points[i].y;
		if (points[i].y < min_y)
			min_y = points[i].y;
	}

	range_y = max_y - min_y + 1;
	int raster[range_y][2];

	for (i = 0; i < range_y; i++) {
		raster[i][0] = -1;
		raster[i][1] = -1;
	}

	for (i = 0; i < num_points; i++) {
		if ((line = bresenham(points[i], points[(i + 1) % num_points], &line_size)) != NULL) {
			printf("line: %p %lu\n", line, line_size);
			printf("line[0]: %i %i\n", line[0].x, line[0].y);
			printf("line[1]: %i %i\n", line[1].x, line[1].y);

			for (j = 0; j < line_size; j++) {
				printf("j: %lu --> %lu\n", j, line_size);
				px = line[j].x;
				py = line[j].y;
				printf("px, py: %i %i\n", px, py);

				if (raster[py][0] < 0 || px < raster[py][0])
					raster[py][0] = px;
				if (raster[py][1] < 0 || px > raster[py][1])
					raster[py][1] = px;
			}

			free(line);
		}
	}

	for (i = 0; i < range_y; i++)
		if (raster[i][0] >= 0 && raster[i][1] >= 0)
			SDL_RenderDrawLine(renderer, raster[i][0], min_y + i, raster[i][1], min_y + i);
}

// bresenham's circle algo adapted from https://web.engr.oregonstate.edu/~sllu/bcircle.pdf
void render_iso_circle(circle_t circle) {
	int x, y;
	int dx, dy;
	int r_err;
	int halfy, halfx;

	// toggle flags are used to render only half of the lines, resulting in ellipse
	bool toggle = false, toggle_flipped = false;
	bool draw_flipped = false;

	x = circle.radius;
	y = 0;
	dx = 1 - (circle.radius << 1);
	dy = 1;
	r_err = 0;

	SDL_RenderDrawLine(renderer, circle.loc.x + x, circle.loc.y,
								 circle.loc.x - x, circle.loc.y);
	
	while (x >= y) {
		if (toggle && y > 1) {
			halfy = y >> 1;
			SDL_RenderDrawLine(renderer, circle.loc.x + x, circle.loc.y + halfy,
										 circle.loc.x - x, circle.loc.y + halfy);
			SDL_RenderDrawLine(renderer, circle.loc.x + x, circle.loc.y - halfy,
										 circle.loc.x - x, circle.loc.y - halfy);

		}
		toggle = !toggle;

		if (draw_flipped && toggle_flipped) {
			halfx = x >> 1;
			SDL_RenderDrawLine(renderer, circle.loc.x + y, circle.loc.y + halfx,
										 circle.loc.x - y, circle.loc.y + halfx);
			SDL_RenderDrawLine(renderer, circle.loc.x + y, circle.loc.y - halfx,
										 circle.loc.x - y, circle.loc.y - halfx);
		}
		draw_flipped = false;

		y++;
		r_err += dy;
		dy += 2;

		if ((r_err << 1) + dx > 0) {
			x--;
			r_err += dx;
			dx += 2;
			draw_flipped = x != y;
			toggle_flipped = !toggle_flipped;
		}
	}
}

void render_aligned_line(v2i start, v2i end) {
	int x, y;
	int dx, dy;
	bool inc_y;

	y = start.y;
	dx = (end.x - start.x >= 0 ? 1 : -1);
	dy = (end.y - start.y >= 0 ? 1 : -1);
	inc_y = false;

	for (x = start.x; x != end.x; x += dx) {	
		SDL_RenderDrawPoint(renderer, x, y);

		if (inc_y)
			y += dy;

		inc_y = !inc_y;
	}
}


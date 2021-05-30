#include <SDL2/SDL.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include "primitives.h"
#include "../render.h"
#include "../lib/vector.h"

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

#ifndef RENDER_PRIMITIVES_H
#define RENDER_PRIMITIVES_H

#include "../vector.h"

struct circle_t {
	v2i loc;
	int radius;
};
typedef struct circle_t circle_t;

//void render_filled_poly(v2i *points, size_t num_points);
void render_iso_circle(circle_t);

#endif

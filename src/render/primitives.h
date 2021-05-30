#ifndef RENDER_PRIMITIVES_H
#define RENDER_PRIMITIVES_H

#include "../lib/vector.h"

struct circle_t {
	v2i loc;
	int radius;
};
typedef struct circle_t circle_t;

void render_iso_circle(circle_t);

#endif

#ifndef RENDER_PRIMITIVES_H
#define RENDER_PRIMITIVES_H

#include "../lib/vector.h"

typedef struct circle {
	v2i loc;
	int radius;
} circle_t;

void render_iso_circle(circle_t);

#endif

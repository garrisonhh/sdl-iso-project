#ifndef RENDER_PRIMITIVES_H
#define RENDER_PRIMITIVES_H

#include "render.h"
#include "collision.h"

typedef bbox_t box_t;

void render_iso_circle(circle_t);
void render_filled_poly(v2i *points, size_t num_points);

#endif

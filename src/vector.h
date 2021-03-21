#ifndef VECTOR_H
#define VECTOR_H

#include <stdbool.h>
#include <SDL2/SDL.h>

// struct SDL_Point { int x, y; };
typedef SDL_Point v2i;

struct v3i {
	int x, y, z;
};
typedef struct v3i v3i;

struct v2d {
	double x, y;
};
typedef struct v2d v2d;

struct v3d {
	double x, y, z;
};
typedef struct v3d v3d;

v2i v2i_add(v2i, v2i);
v2i v2i_sub(v2i, v2i);

double v2d_dot(v2d, v2d);

v3i v3i_from_v3d(v3d);
int v3i_get(v3i *, int);
void v3i_set(v3i *, int, int);
v3i v3i_add(v3i, v3i);
v3i v3i_sub(v3i, v3i);
v3i v3i_scale(v3i, double);
int v3i_compare(v3i, v3i);

v3d v3d_from_v3i(v3i);
double v3d_get(v3d *, int);
void v3d_set(v3d *, int, double);
v3d v3d_add(v3d, v3d);
v3d v3d_sub(v3d, v3d);
v3d v3d_scale(v3d, double);
v3i v3d_polarity(v3d);

#endif

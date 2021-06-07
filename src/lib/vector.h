#ifndef VECTOR_H
#define VECTOR_H

#include <stdbool.h>

struct v2i {
	int x, y;
};
typedef struct v2i v2i;

struct v2d {
	double x, y;
};
typedef struct v2d v2d;

struct v3i {
	int x, y, z;
};
typedef struct v3i v3i;

struct v3d {
	double x, y, z;
};
typedef struct v3d v3d;

// must be called on init
void vector_check_structs(void);

// v2i
#define v2i_IDX(v, idx) ((int *)(&v))[idx]

v2i v2i_from_v2d(v2d);

v2i v2i_add(v2i, v2i);
v2i v2i_sub(v2i, v2i);
v2i v2i_mul(v2i, v2i);
v2i v2i_div(v2i, v2i);

// v2d
#define v2d_IDX(v, idx) ((double *)(&v))[idx]

v2d v2d_from_v2i(v2i);

v2d v2d_add(v2d, v2d);
v2d v2d_sub(v2d, v2d);
v2d v2d_mul(v2d, v2d);
v2d v2d_div(v2d, v2d);

v2d v2d_scale(v2d, double);
v2d v2d_normalize(v2d);
double v2d_magnitude(v2d v);
double v2d_dot(v2d, v2d);

// v3i
#define v3i_IDX(v, idx) ((int *)(&v))[idx]

v3i v3i_from_v3d(v3d);

v3i v3i_add(v3i, v3i);
v3i v3i_sub(v3i, v3i);
v3i v3i_mul(v3i, v3i);
v3i v3i_div(v3i, v3i);

v3i v3i_scale(v3i, double);
v3i v3i_scalei(v3i, int);
double v3i_magnitude(v3i);
double v3i_dot(v3i, v3i);

int v3i_compare(v3i, v3i);
v3i polarity_of_v3d(v3d);

void v3i_print(const char *message, v3i v);
void v3i_sprint(char *string, const char *message, v3i v);

// v3d
#define v3d_IDX(v, idx) ((double *)(&v))[idx]

v3d v3d_from_v3i(v3i);

v3d v3d_add(v3d, v3d);
v3d v3d_sub(v3d, v3d);
v3d v3d_mul(v3d, v3d);
v3d v3d_div(v3d, v3d);

v3d v3d_scale(v3d, double);
v3d v3d_normalize(v3d);
v3d v3d_cross(v3d, v3d);
double v3d_magnitude(v3d);
double v3d_dist(v3d, v3d);
double v3d_dot(v3d, v3d);

int v3d_compare(v3d, v3d);

void v3d_print(const char *message, v3d v);
void v3d_sprint(char *string, const char *message, v3d v);

#endif

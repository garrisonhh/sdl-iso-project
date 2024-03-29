#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <ghh/utils.h>
#include "vector.h"

#define DECL_V2_BASIC(fn_name, fn_type, oper) \
	fn_type fn_name(fn_type a, fn_type b) {\
		return (fn_type){a.x oper b.x, a.y oper b.y};\
	}
#define DECL_V3_BASIC(fn_name, fn_type, oper) \
	fn_type fn_name(fn_type a, fn_type b) {\
		return (fn_type){a.x oper b.x, a.y oper b.y, a.z oper b.z};\
	}

void vector_check_structs() {
	if (!(offsetof(v2i, y) == sizeof(int))) {
		printf("v2i not aligned properly.\n");
		exit(1);
	}

	if (!(offsetof(v2d, y) == sizeof(double))) {
		printf("v2d not aligned properly.\n");
		exit(1);
	}

	if (!(offsetof(v3i, y) == sizeof(int)
	   && offsetof(v3i, z) == sizeof(int) * 2)) {
		printf("v3i not aligned properly.\n");
		exit(1);
	}

	if (!(offsetof(v3d, y) == sizeof(double)
	   && offsetof(v3d, z) == sizeof(double) * 2)) {
		printf("v3d not aligned properly.\n");
		exit(1);
	}
}

// v2i
v2i v2i_from_v2d(v2d v) {
	return (v2i){v.x, v.y};
}

DECL_V2_BASIC(v2i_add, v2i, +)
DECL_V2_BASIC(v2i_sub, v2i, -)
DECL_V2_BASIC(v2i_mul, v2i, *)
DECL_V2_BASIC(v2i_div, v2i, /)

// v2d
v2d v2d_from_v2i(v2i v) {
	return (v2d){v.x, v.y};
}

DECL_V2_BASIC(v2d_add, v2d, +)
DECL_V2_BASIC(v2d_sub, v2d, -)
DECL_V2_BASIC(v2d_mul, v2d, *)
DECL_V2_BASIC(v2d_div, v2d, /)

v2d v2d_scale(v2d v, double scalar) {
	return (v2d){v.x * scalar, v.y * scalar};
}

v2d v2d_normalize(v2d v) {
	return v2d_scale(v, 1.0 / v2d_magnitude(v));
}

double v2d_magnitude(v2d v) {
	return sqrt(v2d_dot(v, v));
}

double v2d_dot(v2d a, v2d b) {
	return a.x * b.x + a.y * b.y;
}

// v3i
v3i v3i_from_v3d(v3d v) {
	return (v3i){v.x, v.y, v.z};
}

DECL_V3_BASIC(v3i_add, v3i, +)
DECL_V3_BASIC(v3i_sub, v3i, -)
DECL_V3_BASIC(v3i_mul, v3i, *)
DECL_V3_BASIC(v3i_div, v3i, /)

v3i v3i_scale(v3i v, double scalar) {
	return (v3i){(double)v.x * scalar, (double)v.y * scalar, (double)v.z * scalar};
}

v3i v3i_scalei(v3i v, int scalar) {
	return (v3i){v.x * scalar, v.y * scalar, v.z * scalar};
}

int v3i_compare(v3i a, v3i b) {
	int v;

	for (int i = 2; i >= 0; --i)
		if ((v = v3i_IDX(a, i) - v3i_IDX(b, i)))
			return v;

	return 0;
}

v3i polarity_of_v3d(v3d v) {
	v3i polarity;

	for (int i = 0; i < 3; i++) {
		if (fequals(v3d_IDX(v, i), 0))
			v3i_IDX(polarity, i) = 0;
		else
			v3i_IDX(polarity, i) = (v3d_IDX(v, i) > 0 ? 1 : -1);
	}

	return polarity;
}

double v3i_magnitude(v3i v) {
	return sqrt(v3i_dot(v, v));
}

double v3i_dot(v3i a, v3i b) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

void v3i_print(const char *message, v3i v) {
	if (message != NULL)
		printf("%s:\t", message);
	printf("{%d\t%d\t%d}\n", v.x, v.y, v.z);
}

void v3i_sprint(char *string, const char *message, v3i v) {
	if (message != NULL)
		sprintf(string, "%s: (%9d, %9d, %9d)", message, v.x, v.y, v.z);
	else
		sprintf(string, "(%9d, %9d, %9d)", v.x, v.y, v.z);
}

// v3d
v3d v3d_from_v3i(v3i v) {
	return (v3d){v.x, v.y, v.z};
}

DECL_V3_BASIC(v3d_add, v3d, +)
DECL_V3_BASIC(v3d_sub, v3d, -)
DECL_V3_BASIC(v3d_mul, v3d, *)
DECL_V3_BASIC(v3d_div, v3d, /)

v3d v3d_scale(v3d v, double scalar) {
	return (v3d){v.x * scalar, v.y * scalar, v.z * scalar};
}

v3d v3d_normalize(v3d v) {
	return v3d_scale(v, 1 / v3d_magnitude(v));
}

v3d v3d_cross(v3d a, v3d b) {
	return (v3d){
		(a.y * b.z) - (b.y * a.z),
		(a.z * b.x) - (b.z * a.x),
		(a.x * b.y) - (b.x * a.y)
	};
}

double v3d_magnitude(v3d v) {
	return sqrt(v3d_dot(v, v));
}

double v3d_dist(v3d a, v3d b) {
	v3d diff = v3d_sub(a, b);

	return sqrt(v3d_dot(diff, diff));
}

double v3d_dot(v3d a, v3d b) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

int v3d_compare(v3d a, v3d b) {
	double v;

	for (int i = 2; i >= 0; --i)
		if (abs((v = v3d_IDX(a, i) - v3d_IDX(b, i))) > EPSILON)
			return (v > 0 ? 1 : -1);

	return 0;
}

void v3d_print(const char *message, v3d v) {
	if (message != NULL)
		printf("%s:\t", message);
	printf("{%9.4f\t%9.4f\t%9.4f}\n", v.x, v.y, v.z);
}

void v3d_sprint(char *string, const char *message, v3d v) {
	if (message != NULL)
		sprintf(string, "%s: (%9.4f, %9.4f, %9.4f)", message, v.x, v.y, v.z);
	else
		sprintf(string, "(%9.4f, %9.4f, %9.4f)", v.x, v.y, v.z);
}

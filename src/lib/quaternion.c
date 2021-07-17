#include <stdio.h>
#include <math.h>
#include <ghh/utils.h>
#include "quaternion.h"

const v3d UNIT_I = {1.0, 0.0, 0.0};
const v3d UNIT_J = {0.0, 1.0, 0.0};
const v3d UNIT_K = {0.0, 0.0, 1.0};

#define DECL_QUAT_BASIC(fn_name, oper) \
	quat_t fn_name(quat_t a, quat_t b) {\
		return (quat_t){a.w oper b.w, a.x oper b.x, a.y oper b.y, a.z oper b.z};\
	}

quat_t quat_from_v3d(v3d v) {
	return (quat_t){0.0, v.x, v.y, v.z};
}

DECL_QUAT_BASIC(quat_add, +)
DECL_QUAT_BASIC(quat_sub, -)
DECL_QUAT_BASIC(quat_mul, *)
DECL_QUAT_BASIC(quat_div, /)

double quat_magnitude(quat_t q) {
	return sqrt(q.w * q.w + q.x * q.x + q.y * q.y + q.z * q.z);
}

quat_t quat_scale(quat_t q, double scalar) {
	return (quat_t){q.w * scalar, q.x * scalar, q.y * scalar, q.z * scalar};
}

quat_t quat_normalize(quat_t q) {
	return quat_scale(q, 1.0 / quat_magnitude(q));
}

quat_t quat_angle_rotation(double angle, v3d about) {
	double sin_a = sin(angle / 2.0);
	quat_t rot = {
		.w = cos(angle / 2.0),
		.x = sin_a * about.x,
		.y = -(sin_a * about.y),
		.z = -(sin_a * about.z)
	};

	return quat_normalize(rot);
}

quat_t quat_vector_rotation(v3d a, v3d b) {
	a = v3d_normalize(a);
	b = v3d_normalize(b);

	quat_t rot = quat_from_v3d(v3d_cross(a, b));

	rot.w = 1.0 + v3d_dot(a, b);

	return quat_normalize(rot);
}

quat_t quat_inverse(quat_t q) {
	return (quat_t){q.w, -q.x, -q.y, -q.z};
}

quat_t quat_hamilton(quat_t a, quat_t b) {
	return (quat_t){
		.w = (a.w * b.w) - (a.x * b.x) - (a.y * b.y) - (a.z * b.z),
		.x = (a.w * b.x) + (a.x * b.w) + (a.y * b.z) - (a.z * b.y),
		.y = (a.w * b.y) - (a.x * b.z) + (a.y * b.w) + (a.z * b.x),
		.z = (a.w * b.z) + (a.x * b.y) - (a.y * b.x) + (a.z * b.w)
	};
}

v3d quat_rotate_v3d(v3d v, quat_t rotation) {
	quat_t p = quat_from_v3d(v);

	p = quat_hamilton(rotation, p);
	p = quat_hamilton(p, quat_inverse(rotation));

	return (v3d){p.x, p.y, p.z};
}

void quat_print(const char *message, quat_t q) {
	if (message != NULL)
		printf("%s:\t", message);
	printf("{%9.4f\t%9.4f\t%9.4f\t%9.4f}\n", q.w, q.x, q.y, q.z);
}

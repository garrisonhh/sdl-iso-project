#include <stdio.h>
#include <math.h>
#include "quaternion.h"
#include "utils.h"

quat_t quat_from_v3d(v3d v) {
	return (quat_t){0.0, v.x, v.y, v.z};
}

quat_t quat_rotation(double angle, v3d about) {
	double sin_a = sin(angle / 2.0);

	return (quat_t){
		.w = cos(angle / 2.0),
		.x = sin_a * about.x,
		.y = sin_a * about.y,
		.z = sin_a * about.z
	};
}

quat_t quat_inverse(quat_t q) {
	return (quat_t){q.w, -q.x, -q.y, -q.z};
}

quat_t quat_hamilton_prod(quat_t a, quat_t b) {
	return (quat_t){
		.w = (a.w * b.w) - (a.x * b.x) - (a.y * b.y) - (a.z * b.z),
		.x = (a.w * b.x) + (a.x * b.w) + (a.y * b.z) - (a.z * b.y),
		.y = (a.w * b.y) - (a.x * b.z) + (a.y * b.w) + (a.z * b.x),
		.z = (a.w * b.z) + (a.x * b.y) - (a.y * b.x) + (a.z * b.w)
	};
}

v3d quat_rotate_v3d(v3d v, quat_t rotation) {
	quat_t p = quat_from_v3d(v);

	p = quat_hamilton_prod(quat_hamilton_prod(rotation, p), quat_inverse(rotation));

	return (v3d){p.x, p.y, p.z};
}

void quat_print(const char *message, quat_t q) {
	if (message != NULL)
		printf("%s:\t", message);
	printf("{%9.4f\t%9.4f\t%9.4f\t%9.4f}\n", q.w, q.x, q.y, q.z);
}

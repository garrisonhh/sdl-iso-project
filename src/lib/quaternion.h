#ifndef QUATERNION_H
#define QUATERNION_H

#include "vector.h"

extern const v3d UNIT_I, UNIT_J, UNIT_K;

typedef struct quat {
	double w, x, y, z;
} quat_t;

quat_t quat_from_v3d(v3d);

quat_t quat_add(quat_t, quat_t);
quat_t quat_sub(quat_t, quat_t);
quat_t quat_mul(quat_t, quat_t);
quat_t quat_div(quat_t, quat_t);

double quat_magnitude(quat_t);
quat_t quat_scale(quat_t, double scalar);
quat_t quat_normalize(quat_t);
quat_t quat_inverse(quat_t);

// quat representation of rotating by an angle around a vector
quat_t quat_angle_rotation(double angle, v3d about);
// quat representation of rotating from a vector to another (normalizes vectors)
quat_t quat_vector_rotation(v3d from, v3d to);
quat_t quat_hamilton(quat_t, quat_t);

v3d quat_rotate_v3d(v3d, quat_t rotation);

void quat_print(const char *message, quat_t);

#endif

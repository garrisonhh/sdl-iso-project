#ifndef QUATERNION_H
#define QUATERNION_H

#include "vector.h"

struct quat_t {
	double w, x, y, z;
};
typedef struct quat_t quat_t;

quat_t quat_from_v3d(v3d);

quat_t quat_rotation(double angle, v3d about);
quat_t quat_inverse(quat_t);
quat_t quat_hamilton_prod(quat_t, quat_t);

v3d quat_rotate_v3d(v3d, quat_t rotation);

void quat_print(const char *message, quat_t);

#endif

#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>
#include <math.h>

/* 
 * apparently M_PI isn't always defined in math.h between c versions?
 * gcc defines M_PI anyways, but it isn't the standard and that
 * would break using another compiler. the c language, dude
 */
#ifndef M_PI
#define M_PI (3.14159265358979323846)
#endif

#define MAX(a, b) (a > b ? a : b)
#define MIN(a, b) (a < b ? a : b)

// most useful god damn macro on the planet holy shit
#define FOR_XYZ(x, y, z, mx, my, mz) for (z = 0; z < mz; z++) for (y = 0; y < my; y++) for (x = 0; x < mx; x++)

bool d_close(double a, double b);

#endif

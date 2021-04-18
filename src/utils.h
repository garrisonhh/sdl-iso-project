#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>
#include <math.h>

/* 
 * apparently M_PI isn't always defined in math.h between c versions?
 * gcc defines M_PI anyways, but it isn't the standard and that
 * would break using another compiler. the c language, dude.
 */
#ifndef M_PI
#define M_PI (3.14159265358979323846)
#endif

#define MAX(a, b) (a > b ? a : b)
#define MIN(a, b) (a < b ? a : b)
#define CLAMP(x, a, b) (MAX(MIN(x, b), a))

#define SWAP(a, b, temp) {\
	temp = a;\
	a = b;\
	b = temp;\
}

// most useful god damn macros on the planet holy guacamole
#define FOR_XY(x, y, mx, my) for (y = 0; y < my; y++) for (x = 0; x < mx; x++)
#define FOR_XYZ(x, y, z, mx, my, mz) for (z = 0; z < mz; z++) for (y = 0; y < my; y++) for (x = 0; x < mx; x++)
#define FOR_CUBE(x, y, z, minv, maxv) for (z = minv; z < maxv; z++) for (y = minv; y < maxv; y++) for (x = minv; x < maxv; x++)

bool d_close(double a, double b);
void timeit_start(void);
void timeit_end(const char *message);
void print_bits(const char *message, unsigned n, size_t bits);

#endif

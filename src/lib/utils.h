#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>
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

#define FLOAT_TOLERANCE 0.000001

#define MAX(a, b) (a > b ? a : b)
#define MIN(a, b) (a < b ? a : b)
#define CLAMP(x, a, b) (MAX(MIN(x, b), a))

#define SWAP(a, b, temp) {\
	temp = a;\
	a = b;\
	b = temp;\
}

#define INRANGE(x, a, b) (x >= a && x < b)

// most useful god damn macros on the planet holy guacamole
#define FOR_XY(x, y, mx, my) for (y = 0; y < my; ++y) for (x = 0; x < mx; ++x)
#define FOR_XYZ(x, y, z, mx, my, mz) for (z = 0; z < mz; ++z) for (y = 0; y < my; ++y) for (x = 0; x < mx; ++x)
#define FOR_CUBE(x, y, z, minv, maxv) for (z = minv; z < maxv; ++z) for (y = minv; y < maxv; ++y) for (x = minv; x < maxv; ++x)

// bitfield ops
#define BIT_GET(bitfield, index) ((bitfield >> (index)) & 1)
#define BIT_SET_TRUE(bitfield, index) {bitfield |= (0x1 << (index));}
#define BIT_SET_FALSE(bitfield, index) {bitfield &= ~(0x1 << (index));}

#define BIT_SET_COND(bitfield, index, cond) {\
	if (cond) \
		BIT_SET_TRUE(bitfield, index) \
	else \
		BIT_SET_FALSE(bitfield, index) \
}

// swp is bool/unsigned/int
#define BIT_SET_SWAP(bitfield, index1, index2, swp) {\
	swp = BIT_GET(bitfield, index1);\
	BIT_SET_COND(bitfield, index1, BIT_GET(bitfield, index2));\
	BIT_SET_COND(bitfield, index2, swp);\
}

#define D_ROUND(d) ((int)(d + 0.5))

extern const float SQRT2;

bool d_close(double a, double b);
double randf(void); // returns values between 0 and 1
double rand_angle(void);
void timeit_start(void);
void timeit_end(const char *message);
double timeit_get_time(void);
void print_bits(const char *message, unsigned n, size_t bits);
void term_set_bg(int color);
void term_set_fg(int color);
void term_reset_color();

#endif

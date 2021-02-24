#include <stdbool.h>
#include <math.h>

#define TOLERANCE 0.00001

bool fisClose(double a, double b) {
	return fabs(a - b) < TOLERANCE;
}

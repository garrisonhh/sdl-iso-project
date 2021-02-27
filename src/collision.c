#include <stdlib.h>
#include <stdbool.h>
#include "collision.h"
#include "vector.h"
#include "entity.h"
#include "world.h"
#include "utils.h"

#include <stdio.h>

bool collides(double a, double b, double x) {
	return (a < x) && (x < b);
}

bool collide1d(double start_a, double len_a, double start_b, double len_b) {
	return collides(start_b, start_b + len_b, start_a)
		|| collides(start_b, start_b + len_b, start_a + len_a)
		|| (d_close(start_a, start_b) && d_close(len_a, len_b));
}

// checks if a is inside b, but not vice versa
// if a surrounds b, no collision will be detected
bool collide_bbox(bbox_t a, bbox_t b) {
	return collide1d(a.pos.x, a.size.x, b.pos.x, b.size.x)
		&& collide1d(a.pos.y, a.size.y, b.pos.y, b.size.y)
		&& collide1d(a.pos.z, a.size.z, b.pos.z, b.size.z);
}

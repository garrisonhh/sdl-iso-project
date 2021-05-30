#include <stdlib.h>
#include "mytimer.h"
#include "utils.h"

struct mytimer_t {
	double last_tick, this_tick;
	double tick;
	double *tracked;
	size_t len_tracked, tracked_idx;
};

mytimer_t *mytimer_create(size_t len_tracked) {
	mytimer_t *timer = malloc(sizeof(mytimer_t));

	timer->this_tick = 0;
	timer->len_tracked = len_tracked;
	timer->tracked_idx = 0;

	timer->tracked = malloc(sizeof(double) * timer->len_tracked);

	for (size_t i = 0; i < timer->len_tracked; ++i)
		timer->tracked[i] = 0.0;

	mytimer_tick(timer);

	return timer;
}

void mytimer_destroy(mytimer_t *timer) {
	free(timer->tracked);
	free(timer);
}

void mytimer_tick(mytimer_t *timer) {
	timer->last_tick = timer->this_tick;
	timer->this_tick = timeit_get_time();

	timer->tick = timer->this_tick - timer->last_tick;

	timer->tracked[timer->tracked_idx++] = timer->tick;
	timer->tracked_idx %= timer->len_tracked;
}

double mytimer_get_tick(mytimer_t *timer) {
	return timer->tick;
}

double mytimer_get_fps(mytimer_t *timer) {
	double average = 0;

	for (size_t i = 0; i < timer->len_tracked; ++i)
		average += timer->tracked[i];

	return timer->len_tracked / average;
}

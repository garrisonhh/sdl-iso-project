#ifndef MYTIMER_H
#define MYTIMER_H

#include <stddef.h>

typedef struct mytimer_t mytimer_t;

mytimer_t *mytimer_create(size_t len_tracked);
void mytimer_destroy(mytimer_t *);

void mytimer_tick(mytimer_t *);
void mytimer_pop_tick(mytimer_t *);
double mytimer_get_tick(mytimer_t *); // time elapsed between last ticks
double mytimer_get_fps(mytimer_t *);

#endif

#ifndef PLANT_H
#define PLANT_H

struct plant_t {
	// growth_rate is per second
	// delta is possible +- rate change for offspring
	double growth_rate, growth_rate_delta;
	double growth;
	int fullgrown;
};
typedef struct plant_t plant_t;

void plant_tick(plant_t *, double time);
int plant_growth_level(plant_t *);

#endif

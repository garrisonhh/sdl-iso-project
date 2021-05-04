#ifndef PLANT_H
#define PLANT_H

struct plant_t {
	// growth_rate is per second
	double growth, growth_rate;
	int fullgrown;
};
typedef struct plant_t plant_t;

void plant_tick(plant_t *, double time);
int plant_growth_level(plant_t *);

#endif

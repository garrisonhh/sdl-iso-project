#ifndef PLANT_H
#define PLANT_H

typedef struct plant {
	// growth_rate is per second
	double growth, growth_rate;
	int fullgrown;
} plant_t;

void plant_tick(plant_t *, double time);
int plant_growth_level(plant_t *);

#endif

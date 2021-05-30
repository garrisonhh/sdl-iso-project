#include <stdlib.h>
#include <stdio.h>
#include "plant.h"
#include "../lib/utils.h"

void plant_tick(plant_t *plant, double time) {
	if (!d_close(plant->growth, plant->fullgrown)) {
		double next_growth = plant->growth + time * plant->growth_rate;	
		plant->growth = MIN(next_growth, plant->fullgrown);
	}
}

// ensures no float errors for texture cell
int plant_growth_level(plant_t *plant) {
	int level = (int)plant->growth;

	return (d_close(plant->growth, level - 1) ? level - 1: level);
}

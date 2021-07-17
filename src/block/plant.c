#include <stdlib.h>
#include <stdio.h>
#include <ghh/utils.h>
#include "plant.h"

void plant_tick(plant_t *plant, double time) {
	if (!fequals(plant->growth, plant->fullgrown)) {
		double next_growth = plant->growth + time * plant->growth_rate;
		plant->growth = MIN(next_growth, plant->fullgrown);
	}
}

// ensures no float errors for texture cell
int plant_growth_level(plant_t *plant) {
	int level = (int)plant->growth;

	return (fequals(plant->growth, level - 1) ? level - 1: level);
}

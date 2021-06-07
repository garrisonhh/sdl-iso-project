#ifndef WORLD_GENERATE_H
#define WORLD_GENERATE_H

typedef struct world_t world_t;

enum world_gen_type_e {
	WORLD_NORMAL,
	WORLD_FLAT,
	WORLD_ALIEN,

	NUM_WORLD_TYPES
};
typedef enum world_gen_type_e world_gen_type_e;

void world_generate(world_t *, world_gen_type_e);

#endif

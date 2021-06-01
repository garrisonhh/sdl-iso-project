#ifndef GENERATE_H
#define GENERATE_H

typedef struct world_t world_t;

enum world_gen_type_e {
	WORLD_FLAT,
	WORLD_ALIEN,
};
typedef enum world_gen_type_e world_gen_type_e;

void world_generate(world_t *, world_gen_type_e);

#endif

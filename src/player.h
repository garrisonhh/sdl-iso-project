#ifndef PLAYER_H
#define PLAYER_H

#include "vector.h"

typedef struct world_t world_t;

void player_init(world_t *);

void player_tick(void);

v3d player_get_pos(void);

#endif

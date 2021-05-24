#ifndef PLAYER_H
#define PLAYER_H

#include "vector.h"

typedef struct world_t world_t;

extern bool GODMODE;

void player_init(world_t *);

void player_tick(void);

v3d player_get_pos(void);
void player_toggle_godmode(void);

#endif

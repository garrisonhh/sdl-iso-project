#ifndef PLAYER_H
#define PLAYER_H

#include "lib/vector.h"

typedef struct world_t world_t;

extern bool GODMODE;

void player_init(world_t *);

void player_tick(void);

void player_click(world_t *, v2i);
void player_toggle_godmode(void);

v3d player_get_pos(void);
v3d player_get_head_pos(void);

#endif

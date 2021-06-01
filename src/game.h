#ifndef GAME_H
#define GAME_H

#include <SDL2/SDL.h>
#include "world.h"

// used by new world menu
extern int NEW_WORLD_SIZE;
extern world_gen_type_e WORLD_TYPE;

void game_init(void);

void game_main(void);

#endif

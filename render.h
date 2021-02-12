#include <SDL2/SDL.h>
#include "world.h"

#ifndef RENDER_H
#define RENDER_H

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 800

extern SDL_Renderer *renderer;

void initRenderer(SDL_Window *);
void destroyRenderer(void);
void renderWorld(world_t *);

#endif

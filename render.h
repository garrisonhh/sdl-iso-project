#include <SDL2/SDL.h>
#include "world.h"

#ifndef RENDER_H
#define RENDER_H

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 800
#define VOXEL_WIDTH 32
#define VOXEL_HEIGHT 34

extern const int VOXEL_Z_HEIGHT;
extern SDL_Renderer *renderer;

SDL_Texture *loadSDLTexture(char *path);
void initRenderer(SDL_Window *);
void destroyRenderer(void);
void loadMedia(void);
void destroyMedia(void);
void renderWorld(world_t *);
void updateCamera();

#endif

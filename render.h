#ifndef RENDER_H
#define RENDER_H

#include <SDL2/SDL.h>
#include "world.h"

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 800
#define VOXEL_WIDTH 32
#define VOXEL_HEIGHT 34

extern const int VOXEL_Z_HEIGHT;
extern const v2i SCREEN_CENTER;
extern v2i camera;

extern SDL_Renderer *renderer;

SDL_Texture *loadSDLTexture(char *path);
void initRenderer(SDL_Window *);
void destroyRenderer(void);
void loadMedia(void);
void destroyMedia(void);
void updateCamera(world_t *);
void renderWorld(world_t *);

#endif

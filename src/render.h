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

SDL_Texture *load_sdl_texture(char *path);
void render_init(SDL_Window *);
void render_destroy(void);
void media_load(void);
void media_destroy(void);
void update_camera(world_t *);
void render_world(world_t *);

#endif
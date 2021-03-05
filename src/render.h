#ifndef RENDER_H
#define RENDER_H

#include <SDL2/SDL.h>
#include "world.h"

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 800
#define VOXEL_WIDTH 32
#define VOXEL_HEIGHT 34

extern const int VOXEL_Z_HEIGHT;

extern v2i camera;
extern int camera_scale;

extern SDL_Renderer *renderer;

void render_init(SDL_Window *);
void render_destroy(void);
void render_clear_screen(void);
void camera_update(world_t *);
void camera_set_scale(int);
void render_world(world_t *);

#endif

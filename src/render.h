#ifndef RENDER_H
#define RENDER_H

#include <SDL2/SDL.h>
#include <stdbool.h>
#include "world.h"

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 800
#define VOXEL_WIDTH 32
#define VOXEL_HEIGHT 34

struct circle_t {
	v2i loc;
	int radius;
};
typedef struct circle_t circle_t;

extern const int VOXEL_Z_HEIGHT;
extern SDL_Renderer *renderer;

void render_init(SDL_Window *);
void render_destroy(void);
void render_clear_screen(void);
void render_world(world_t *);

#endif

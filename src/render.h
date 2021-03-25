#ifndef RENDER_H
#define RENDER_H

#include <SDL2/SDL.h>
#include <stdbool.h>
#include "world.h"

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 800
#define VOXEL_WIDTH 32
#define VOXEL_HEIGHT 34

struct camera_t {
	int scale, render_dist;
	v2i pos, center_screen;
	SDL_Rect viewport;
};
typedef struct camera_t camera_t;

extern const int VOXEL_Z_HEIGHT;
extern SDL_Renderer *renderer;
extern camera_t camera;

void render_init(SDL_Window *);
void render_destroy(void);
void render_clear_screen(void);
void camera_update(world_t *);
void camera_set_scale(int);
void camera_change_scale(bool);
void render_world(world_t *);

#endif

#ifndef CAMERA_H
#define CAMERA_H

#include "SDL2/SDL.h"
#include "world.h"
#include "render.h"
#include "render_primitives.h"

struct camera_t {
	// position
	v3d center; // position in world
	v2i pos; // position on screen

	// view
	int scale;
	v2i center_screen;
	SDL_Rect viewport;
	circle_t view_circle;

	// rendering
	int render_dist, block_size;
	v3i min_render, max_render, inc_render; // render_world iteration values
	int rotation; // 0-3; whether facing N, E, S, W
};
typedef struct camera_t camera_t;

extern camera_t camera;

v2i project_v3i(v3i);
v2i project_v3d(v3d);

void camera_init(void);

void camera_set_block_size(int);
void camera_set_center(v3d);
void camera_set_scale(int);
void camera_change_scale(bool);
void camera_rotate(bool);

#endif

#ifndef CAMERA_H
#define CAMERA_H

#include "SDL2/SDL.h"
#include "world.h"
#include "render.h"
#include "render_primitives.h"

struct camera_t {
	int scale, render_dist;
	v2i pos, center_screen;
	SDL_Rect viewport;
	circle_t view_circle;

	int rotation; // 0-3; whether facing N, E, S, W
};
typedef struct camera_t camera_t;

extern camera_t camera;

v2i project_v3i(v3i);
v2i project_v3d(v3d);

void camera_init(void);

void camera_update(world_t *);
void camera_set_scale(int);
void camera_change_scale(bool);

#endif

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
};
typedef struct camera_t camera_t;

extern camera_t camera;

void camera_init(void);
v2i project_v3i(v3i, bool);
v2i project_v3d(v3d, bool);
void camera_update(world_t *);
void camera_set_scale(int);
void camera_change_scale(bool);

#endif

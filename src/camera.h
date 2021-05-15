#ifndef CAMERA_H
#define CAMERA_H

#include "SDL2/SDL.h"
#include "world.h"
#include "render.h"
#include "render_primitives.h"

struct camera_t {
	// position
	v3d pos; // position in world
	v2i center; // projected position relative to (0, 0, 0) 

	// view
	int scale;
	v2i center_screen;
	SDL_Rect viewport;
	circle_t view_circle;

	// rendering
	int render_dist, block_size;
	v3i render_start, render_end, render_inc; // render_world iteration values
	int rotation; // 0-3; whether facing N, E, S, W
};
typedef struct camera_t camera_t;

extern camera_t camera;

v2i project_v3i(v3i);
v2i project_v3d(v3d);

void camera_init(void);

v3d camera_rotated_v3d(v3d);
v3d camera_reverse_rotated_v3d(v3d);
v3i camera_rotated_v3i(v3i);
v3i camera_reverse_rotated_v3i(v3i);
void camera_set_block_size(int);
void camera_set_pos(v3d);
void camera_set_scale(int);
void camera_change_scale(bool);
void camera_rotate(bool);

#endif

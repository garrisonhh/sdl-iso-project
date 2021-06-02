#ifndef CAMERA_H
#define CAMERA_H

#include "SDL2/SDL.h"
#include "world.h"
#include "render.h"
#include "render/primitives.h"

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
	int rotation; // 0-3; cardinal directions
	v3i facing;
	v3d view_dir;
	v3i world_limits, render_center, render_limits;
};
typedef struct camera_t camera_t;

extern camera_t camera;

void camera_init(void);

// call before voxel raycasting
void camera_update_limits(int block_size);
void camera_set_pos(v3d);
void camera_set_scale(int);
void camera_scale(bool);
void camera_rotate(bool);

v2i project_v3i(v3i);
v2i project_v3d(v3d);
v3d un_project(v2i, double z);

v3d camera_rotated_v3d(v3d);
v3d camera_reverse_rotated_v3d(v3d);
v3i camera_rotated_v3i(v3i);
v3i camera_reverse_rotated_v3i(v3i);

#endif

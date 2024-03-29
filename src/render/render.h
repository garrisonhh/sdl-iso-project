#ifndef RENDER_H
#define RENDER_H

#include <SDL2/SDL.h>
#include <stdbool.h>
#include <ghh/array.h>
#include "packet.h"

#define RENDER_FORMAT SDL_PIXELFORMAT_RGBA8888

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 800

#define VOXEL_WIDTH 32
#define VOXEL_HEIGHT 34
#define VOXEL_Z_HEIGHT (VOXEL_HEIGHT - (VOXEL_WIDTH >> 1))

#define BG_GRAY 0x1F

extern SDL_Renderer *RENDERER;

typedef struct world world_t;

typedef struct render_info {
	array_t *bg_packets, *fg_packets;
	bool cam_hit;
	int z_split;

	SDL_Rect cam_viewport;
} render_info_t;

void render_init(SDL_Window *);
void render_game_init(void);
void render_game_quit(void);
void render_quit(void);

render_info_t *render_gen_info(world_t *world);
void render_info_destroy(render_info_t *);

void render_from_info(render_info_t *);

#endif

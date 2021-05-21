#include <SDL2/SDL.h>
#include <stdbool.h>
#include <math.h>
#include "fonts.h"
#include "textures.h"
#include "../render.h"
#include "../camera.h"
#include "../player.h"
#include "../textures.h"
#include "../world.h"
#include "../utils.h"

const int GUI_WIDTH = SCREEN_WIDTH >> 2;
const int GUI_HEIGHT = SCREEN_HEIGHT >> 2;
SDL_Texture *STATIC_GUI;

bool UPDATE_STATIC = true; // gui_render will only re-draw static gui elements if this is true
bool DEBUG = false;

sprite_t *COMPASS;
v2i COMPASS_POS, COMPASS_CELL;

#define NUM_DEBUG_LINES 10
char DEBUG_LINES[NUM_DEBUG_LINES][64];

// call after render_init
void gui_init() {
	STATIC_GUI = SDL_CreateTexture(renderer,
							RENDER_FORMAT,
							SDL_TEXTUREACCESS_TARGET,
							GUI_WIDTH, GUI_HEIGHT);
	SDL_SetTextureBlendMode(STATIC_GUI, SDL_BLENDMODE_BLEND);

	for (int i = 0; i < NUM_DEBUG_LINES; ++i)
			DEBUG_LINES[i][0] = '\0';
}

// call after textures_load
void gui_load() {
	COMPASS = sprite_from_key("compass");
	COMPASS_POS = (v2i){GUI_WIDTH - COMPASS->size.x, 0};
	COMPASS_CELL = (v2i){0, 0};
}

void gui_update(double fps, int packets, world_t *world) {
	int line = 0;

	sprintf(DEBUG_LINES[line++], "FPS: %3d", D_ROUND(fps));

	if (COMPASS_CELL.x != camera.rotation) {
		COMPASS_CELL.x = camera.rotation;
		UPDATE_STATIC = true;
	}

	v3d_sprint(DEBUG_LINES[line++], "POS", player_get_pos());
	sprintf(DEBUG_LINES[line++], "ROTATION: %i", camera.rotation);
	sprintf(DEBUG_LINES[line++], "RNDR DIST: %i", camera.rndr_dist);
	sprintf(DEBUG_LINES[line++], "PACKETS: %i", packets);
}

void gui_render() {
	if (UPDATE_STATIC) {
		SDL_SetRenderTarget(renderer, STATIC_GUI);

		SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
		SDL_RenderClear(renderer);

		render_sprite_no_offset(COMPASS, COMPASS_POS, COMPASS_CELL); 
		
		SDL_SetRenderTarget(renderer, NULL);
		UPDATE_STATIC = false;
	}

	SDL_RenderCopy(renderer, STATIC_GUI, NULL, NULL);

	if (DEBUG) {
		v2i line_start = {0, 0};

		for (int i = 0; i < NUM_DEBUG_LINES; ++i) {
			fonts_render_text(FONT_UI, DEBUG_LINES[i], line_start);

			line_start.y += FONTS[FONT_UI].char_size.y;
		}
	}
}

void gui_toggle_debug() {
	DEBUG = !DEBUG;
}

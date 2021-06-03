#include <SDL2/SDL.h>
#include <stdbool.h>
#include <math.h>
#include "gui.h"
#include "fonts.h"
#include "textures.h"
#include "../render.h"
#include "../camera.h"
#include "../player.h"
#include "../sprites.h"
#include "../world.h"
#include "../lib/utils.h"

const int GUI_WIDTH = SCREEN_WIDTH >> 2;
const int GUI_HEIGHT = SCREEN_HEIGHT >> 2;
SDL_Texture *STATIC_GUI;

bool UPDATE_STATIC = true; // gui_render will only re-draw static gui elements if this is true
bool DISP_DEBUG = false;
bool DEBUG_EXEC = false;

sprite_t *COMPASS;
v2i COMPASS_POS, COMPASS_CELL;

#define NUM_DEBUG_LINES 10
char DEBUG_LINES[NUM_DEBUG_LINES][64];

gui_data_t GUI_DATA;

// call after render_init
void gui_init() {
#ifdef DEBUG
	DEBUG_EXEC = true;
#endif

	STATIC_GUI = SDL_CreateTexture(RENDERER,
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

void gui_tick() {
	int line = 0;

	if (COMPASS_CELL.x != camera.rotation) {
		COMPASS_CELL.x = camera.rotation;
		UPDATE_STATIC = true;
	}

	sprintf(DEBUG_LINES[line++], "rendering %i packets @ %3d FPS", GUI_DATA.packets, D_ROUND(GUI_DATA.fps));
	sprintf(DEBUG_LINES[line++], "BUILD: %s", (DEBUG_EXEC ? "debug" : "release"));
	v3d_sprint(DEBUG_LINES[line++], "POSITION", player_get_pos());
	sprintf(DEBUG_LINES[line++], "ROTATION: %i", camera.rotation);
	sprintf(DEBUG_LINES[line++], "GODMODE: %s", (GODMODE ? "on" : "off"));
}

void gui_render() {
	if (UPDATE_STATIC) {
		SDL_SetRenderTarget(RENDERER, STATIC_GUI);

		SDL_SetRenderDrawColor(RENDERER, 0x00, 0x00, 0x00, 0x00);
		SDL_RenderClear(RENDERER);

		render_sprite_no_offset(COMPASS, COMPASS_POS, COMPASS_CELL); 
		
		SDL_SetRenderTarget(RENDERER, NULL);
		UPDATE_STATIC = false;
	}

	SDL_RenderCopy(RENDERER, STATIC_GUI, NULL, NULL);

	if (DISP_DEBUG) {
		v2i line_start = {0, 0};

		for (int i = 0; i < NUM_DEBUG_LINES; ++i) {
			font_render(FONT_UI, DEBUG_LINES[i], line_start);

			line_start.y += font_line_height(FONT_UI);
		}
	}
}

void gui_toggle_debug() {
	DISP_DEBUG = !DISP_DEBUG;
}

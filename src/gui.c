#include <SDL2/SDL.h>
#include <stdbool.h>
#include "render.h"
#include "textures.h"
#include "render_textures.h"
#include "camera.h"

const int GUI_WIDTH = SCREEN_WIDTH >> 2;
const int GUI_HEIGHT = SCREEN_HEIGHT >> 2;
SDL_Texture *STATIC_GUI;

bool UPDATED_FLAG = true; // gui_render will only re-draw static gui elements if this is true

sprite_t *COMPASS;
v2i COMPASS_POS, COMPASS_CELL;

// call after render_init
void gui_init() {
	STATIC_GUI = SDL_CreateTexture(renderer,
							SDL_PIXELFORMAT_RGBA8888,
							SDL_TEXTUREACCESS_TARGET,
							GUI_WIDTH, GUI_HEIGHT);
	SDL_SetTextureBlendMode(STATIC_GUI, SDL_BLENDMODE_BLEND);
}

// call after textures_load
void gui_load() {
	COMPASS = sprite_from_key("compass");
	COMPASS_POS = (v2i){GUI_WIDTH - COMPASS->size.x, 0};
	COMPASS_CELL = (v2i){0, 0};
}

void gui_update(double fps) {
	// TODO
}

void gui_render() {
	if (UPDATED_FLAG) {
		SDL_SetRenderTarget(renderer, STATIC_GUI);

		SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
		SDL_RenderClear(renderer);

		render_sprite_no_offset(COMPASS, COMPASS_POS, COMPASS_CELL); 
		
		SDL_SetRenderTarget(renderer, NULL);
		UPDATED_FLAG = false;
	}

	SDL_RenderCopy(renderer, STATIC_GUI, NULL, NULL);
}
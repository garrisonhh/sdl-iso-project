#ifndef FONTS_H
#define FONTS_H

#include <SDL2/SDL.h>
#include "../lib/vector.h"

enum font_e {
	FONT_UI,
	FONT_MENU,

	NUM_FONTS
};
typedef enum font_e font_e;

void fonts_load(void);

v2i font_char_size(font_e type);
int font_line_height(font_e type);

void font_render(font_e type, const char *text, v2i pos);
SDL_Texture *font_render_static(font_e type, const char *text);

#endif

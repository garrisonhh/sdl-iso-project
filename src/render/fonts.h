#ifndef FONTS_H
#define FONTS_H

#include <SDL2/SDL.h>
#include "../vector.h"

enum font_e {
	FONT_UI = 0,
};
typedef enum font_e font_e;

struct font_t {
	SDL_Texture *sheet;
	v2i char_size, sheet_size;
};
typedef struct font_t font_t;

extern font_t FONTS[1]; // corresponds to font_e members

void fonts_load(void);

void fonts_render_text(font_e type, const char *text, v2i pos);

#endif

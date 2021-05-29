#ifndef FONTS_H
#define FONTS_H

#include <SDL2/SDL.h>
#include "../vector.h"

enum font_e {
	FONT_UI = 0,
	FONT_MENU = 1,
};
typedef enum font_e font_e;

void fonts_load(void);

v2i font_char_size(font_e type);
int font_line_height(font_e type);

void font_render_text(font_e type, const char *text, v2i pos);

#endif

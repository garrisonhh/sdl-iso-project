#ifndef FONTS_H
#define FONTS_H

#include "vector.h"

enum font_e {
	FONT_UI = 0,
};
typedef enum font_e font_e;

void fonts_load(void);

void fonts_render_text(font_e type, const char *text, v2i pos);

#endif

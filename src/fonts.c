#include <SDL2/SDL.h>
#include <json-c/json.h>
#include <stdlib.h>
#include <string.h>
#include "fonts.h"
#include "content.h"
#include "render.h"
#include "textures.h"
#include "vector.h"

font_t FONTS[1]; // corresponds to font_e

font_t load_font(json_object *font_obj) {
	char font_path[100] = "assets/";

	strcat(font_path, content_get_string(font_obj, "path"));

	return (font_t){
		.sheet = load_sdl_texture(font_path),
		.char_size = content_get_v2i(font_obj, "char-size"),
		.sheet_size = content_get_v2i(font_obj, "sheet-size")
	};
}

void fonts_load() {
	json_object *file_obj = content_load_file("assets/fonts.json");

	FONTS[FONT_UI] = load_font(content_get_obj(file_obj, "ui"));

	content_close_file(file_obj);
}

// TODO static text rendering (generate a new texture)

void fonts_render_text(font_e type, const char *text, v2i pos) {
	div_t divided;
	size_t i = 0;

	SDL_Rect src_rect = {
		0,
		0,
		FONTS[type].char_size.x,
		FONTS[type].char_size.y
	};
	SDL_Rect dst_rect = {
		pos.x,
		pos.y,
		FONTS[type].char_size.x,
		FONTS[type].char_size.y
	};

	while (text[i]) {
		divided = div(text[i++], FONTS[type].sheet_size.x);

		src_rect.x = divided.rem * FONTS[type].char_size.x;
		src_rect.y = divided.quot * FONTS[type].char_size.y;

		SDL_RenderCopy(renderer, FONTS[type].sheet, &src_rect, &dst_rect);

		dst_rect.x += FONTS[type].char_size.x;
	}
}

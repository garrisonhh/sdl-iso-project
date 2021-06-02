#include <SDL2/SDL.h>
#include <json-c/json.h>
#include <stdlib.h>
#include <string.h>
#include "fonts.h"
#include "../render.h"
#include "../content.h"
#include "../textures.h"
#include "../lib/vector.h"

struct font_t {
	SDL_Texture *sheet;
	v2i char_size, sheet_size;
	int scale, margin;
};
typedef struct font_t font_t;

font_t FONTS[NUM_FONTS];

font_t load_font(json_object *font_obj) {
	char font_path[100] = "assets/";
	v2i char_size, sheet_size;
	SDL_Texture *sheet;
	int scale = 1, margin = 2;

	strcat(font_path, content_get_string(font_obj, "path"));

	sheet = load_sdl_texture(font_path);
	char_size = content_get_v2i(font_obj, "char-size");
	
	SDL_QueryTexture(sheet, NULL, NULL, &sheet_size.x, &sheet_size.y);

	sheet_size = v2i_div(sheet_size, char_size);

	if (content_has_key(font_obj, "scale"))
		scale = content_get_int(font_obj, "scale");

	if (content_has_key(font_obj, "margin"))
		margin = content_get_int(font_obj, "margin");

	return (font_t){sheet, char_size, sheet_size, scale, margin};
}

void fonts_load() {
	json_object *file_obj = content_load_file("assets/fonts.json");

	const char *font_names[NUM_FONTS] = {
		"ui",
		"menu"
	};

	for (int i = 0; i < NUM_FONTS; ++i)
		FONTS[i] = load_font(content_get_obj(file_obj, font_names[i]));

	content_close_file(file_obj);
}

v2i font_char_size(font_e type) {
	return (v2i){
		FONTS[type].char_size.x * FONTS[type].scale,
		FONTS[type].char_size.y * FONTS[type].scale,
	};
}

int font_line_height(font_e type) {
	return FONTS[type].char_size.y * FONTS[type].scale + FONTS[type].margin * 2;
}

void font_render(font_e type, const char *text, v2i pos) {
	div_t divided;
	size_t i = 0;
	v2i char_size = font_char_size(type);

	SDL_Rect src_rect = {
		0, 0,
		FONTS[type].char_size.x, FONTS[type].char_size.y
	};
	SDL_Rect dst_rect = {
		pos.x + FONTS[type].margin, pos.y + FONTS[type].margin,
		char_size.x, char_size.y
	};

	while (text[i]) {
		divided = div(text[i++], FONTS[type].sheet_size.x);

		src_rect.x = divided.rem * FONTS[type].char_size.x;
		src_rect.y = divided.quot * FONTS[type].char_size.y;

		SDL_RenderCopy(RENDERER, FONTS[type].sheet, &src_rect, &dst_rect);

		dst_rect.x += char_size.x;
	}
}

SDL_Texture *font_render_static(font_e type, const char *text) {
	v2i char_size = font_char_size(type);
	size_t text_len = strlen(text);
	v2i text_size = {
		char_size.x * text_len + FONTS[type].margin * 2,
		char_size.y + FONTS[type].margin * 2
	};
	v2i pos = {0, 0};

	SDL_Texture *texture = SDL_CreateTexture(RENDERER, RENDER_FORMAT,
											 SDL_TEXTUREACCESS_TARGET,
											 text_size.x, text_size.y);
	SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

	SDL_SetRenderTarget(RENDERER, texture);
	SDL_SetRenderDrawColor(RENDERER, 0x00, 0x00, 0x00, 0x00);
	SDL_RenderClear(RENDERER);

	font_render(type, text, pos);

	SDL_SetRenderTarget(RENDERER, NULL);

	return texture;
}

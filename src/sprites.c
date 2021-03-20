#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <json-c/json.h>
#include "vector.h"
#include "render.h"
#include "media.h"
#include "sprites.h"

sprite_t **sprites = NULL;
size_t num_sprites;

void sprites_load(json_object *file_obj) {
	json_object *sprite_arr_obj;
	sprite_arr_obj = json_object_object_get(file_obj, "sprites");
	num_sprites = json_object_array_length(sprite_arr_obj);
	sprites = (sprite_t **)calloc(num_sprites, sizeof(sprite_t *));

	for (size_t i = 0; i < num_sprites; i++) {
		json_object *current_png;
		char path[50];

		current_png = json_object_array_get_idx(sprite_arr_obj, i);
		sprintf(path, "assets/%s.png", json_object_get_string(json_object_object_get(current_png, "name")));

		sprites[i] = (sprite_t *)malloc(sizeof(sprite_t));
		sprites[i]->texture = load_sdl_texture(path);
	
		SDL_QueryTexture(sprites[i]->texture, NULL, NULL, &(sprites[i]->size.x), &(sprites[i]->size.y));
		sprites[i]->pos = (v2i){-(sprites[i]->size.x >> 1), -sprites[i]->size.y};
	}
}

void sprites_destroy() {
	for (size_t i = 0; i < num_sprites; i++) {
		SDL_DestroyTexture(sprites[i]->texture);
		free(sprites[i]);
	}

	free(sprites);
	sprites = NULL;
}

void render_sprite(sprite_t *sprite, v2i pos) {
	SDL_Rect draw_rect = {
		pos.x + sprite->pos.x,
		pos.y + sprite->pos.y,
		sprite->size.x,
		sprite->size.y
	};
	SDL_RenderCopy(renderer, sprite->texture, NULL, &draw_rect);
}

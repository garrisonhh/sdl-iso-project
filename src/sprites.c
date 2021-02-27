#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <json-c/json.h>
#include "render.h"
#include "sprites.h"

sprite_t **sprites = NULL;
int num_sprites;

void sprites_load() {
	json_object *sprite_arr_obj;
	sprite_arr_obj = json_object_object_get(json_object_from_file("assets/assets.json"), "sprites");
	num_sprites = json_object_array_length(sprite_arr_obj);
	sprites = (sprite_t **)calloc(num_sprites, sizeof(sprite_t *));

	for (int i = 0; i < num_sprites; i++) {
		json_object *current_png;
		char path[50];
		current_png = json_object_array_get_idx(sprite_arr_obj, i);
		sprintf(path, "assets/%s.png", json_object_get_string(json_object_object_get(current_png, "name")));

		sprites[i] = (sprite_t *)malloc(sizeof(sprite_t));
		sprites[i]->texture = load_sdl_texture(path);
	
		SDL_QueryTexture(sprites[i]->texture, NULL, NULL, &sprites[i]->w, &sprites[i]->h);
		sprites[i]->x = -(sprites[i]->w >> 1);
		sprites[i]->y = -sprites[i]->h;
	}
}

void sprites_destroy() {
	for (int i = 0; i < num_sprites; i++) {
		SDL_DestroyTexture(sprites[i]->texture);
		free(sprites[i]);
		sprites[i] = NULL;
	}
	free(sprites);
	sprites = NULL;
}

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <json-c/json.h>
#include "render.h"
#include "sprites.h"

sprite_t **sprites = NULL;
int numSprites;

void loadSprites() {
	json_object *spriteArrayObj;
	spriteArrayObj = json_object_object_get(json_object_from_file("assets/assets.json"), "sprites");
	numSprites = json_object_array_length(spriteArrayObj);
	sprites = (sprite_t **)calloc(numSprites, sizeof(sprite_t *));

	for (int i = 0; i < numSprites; i++) {
		json_object *currentTex;
		char path[50];
		currentTex = json_object_array_get_idx(spriteArrayObj, i);
		sprintf(path, "assets/%s.png", json_object_get_string(json_object_object_get(currentTex, "name")));

		sprites[i] = (sprite_t *)malloc(sizeof(sprite_t));
		sprites[i]->texture = loadSDLTexture(path);
	
		SDL_QueryTexture(sprites[i]->texture, NULL, NULL, &sprites[i]->w, &sprites[i]->h);
		sprites[i]->offsetX = -(sprites[i]->w >> 1);
		sprites[i]->offsetY = -sprites[i]->h;
	}
}

void destroySprites() {
	for (int i = 0; i < numSprites; i++) {
		SDL_DestroyTexture(sprites[i]->texture);
		free(sprites[i]);
		sprites[i] = NULL;
	}
	free(sprites);
	sprites = NULL;
}

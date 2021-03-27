#include <SDL2/SDL.h>
#include <json-c/json.h>
#include "textures.h"
#include "block_gen.h"

void content_init() {
	json_object *file_obj = json_object_from_file("assets/content.json");
	
	textures_init();
	textures_load(file_obj);
	block_gen_load(file_obj);
	
	while (json_object_put(file_obj) != 1)
		free(file_obj);
}

void content_quit() {
	textures_destroy();
	block_gen_destroy();
}

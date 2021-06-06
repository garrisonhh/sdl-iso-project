#ifndef RENDER_TEXTURES_H
#define RENDER_TEXTURES_H

#include <SDL2/SDL.h>
#include "../textures.h"
#include "../sprites.h"
#include "../lib/vector.h"
#include "../world/masks.h"

void render_sprite(sprite_t *sprite, v2i, v2i cell);
void render_tex_texture(texture_t *, v2i);
void render_voxel_texture(texture_t *, v2i, voxel_masks_t masks);
void render_connected_texture(texture_t *, v2i, unsigned connected_mask);
void render_sheet_texture(texture_t *, v2i, v2i cell);

void render_sprite_no_offset(sprite_t *sprite, v2i, v2i cell);
SDL_Surface *render_cached_voxel_surface(SDL_Surface *surfaces[3]);

#endif

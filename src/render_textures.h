#ifndef RENDER_TEXTURES_H
#define RENDER_TEXTURES_H

#include <SDL2/SDL.h>
#include "vector.h"
#include "render.h"

void render_textures_init(void);

void render_sdl_texture(SDL_Texture *, v2i);
void render_sprite(sprite_t *sprite, v2i, v2i cell);
void render_sprite_no_offset(sprite_t *sprite, v2i, v2i cell);
void render_voxel_texture(voxel_tex_t *, v2i, unsigned expose_mask, unsigned void_mask);
void render_connected_texture(connected_tex_t *, v2i, unsigned connected_mask);
void render_sheet_texture(sheet_tex_t *, v2i, v2i cell);

#endif

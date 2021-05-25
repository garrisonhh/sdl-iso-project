#!/usr/bin/env python

import pygame as pg

W = 25
H = 25
SPLIT = 13

# 0 = front, 1 = back, 2 = split 
SPRITE_MODES = (
    (0, 0, 0, 0, 0, 0),
    (1, 0, 0, 0, 0, 0),
    (0, 0, 0, 0, 0, 0),
    (0, 0, 0, 0, 0, 0),
    
    (0, 0, 2, 2, 0, 0),
    (0, 0, 0, 0, 0, 0),
    (0, 0, 0, 0, 0, 0),
    (0, 0, 0, 0, 1, 1),

    (1, 1, 1, 1, 1, 1),
    (1, 1, 1, 1, 1, 1),
    (0, 1, 1, 1, 2, 2),
    (0, 0, 1, 2, 2, 2),
)

def cell_rect(x, y):
    return (x * W, y * H, W, H)

def copy_cell(dst, dst_x, dst_y, src, src_x, src_y):
    dst.blit(src.subsurface(cell_rect(src_x, src_y)), (dst_x * W, dst_y * H))

def cut_cell(dst, dst_x, dst_y, src, src_x, src_y):
    copy_cell(dst, dst_x, dst_y, src, src_x, src_y)

    src.fill((0, 0, 0, 0), cell_rect(src_x, src_y), special_flags = pg.BLEND_RGBA_MIN)

def cut_rect(dst, src, rect):
    dst.blit(src.subsurface(rect), rect)
    src.fill((0, 0, 0, 0), rect, special_flags = pg.BLEND_RGBA_MIN)

def main():
    pg.init()
    pg.display.set_mode((0, 0))

    axe_swing = pg.image.load("axe_swing.png")
    axe_fast = pg.image.load("axe_swing_fast.png")
    axe_fastest = pg.image.load("axe_swing_fastest.png")
    axe_unmoving = pg.image.load("axe_unmoving.png")

    front = pg.Surface((W * len(SPRITE_MODES[0]), H * len(SPRITE_MODES))).convert_alpha()
    front.fill((0, 0, 0, 0))
    front.blit(axe_unmoving, (0, 0))

    for i in range(8):
        copy_cell(front, 0, i + 4, axe_swing, (i + 2) % 8, 0)
        copy_cell(front, 1, i + 4, axe_swing, (i + 1) % 8, 0)
        copy_cell(front, 2, i + 4, axe_swing, i, 0)
        copy_cell(front, 3, i + 4, axe_fast, (i + 1) % 8, 0)
        copy_cell(front, 4, i + 4, axe_fastest, (i + 3) % 8, 0)
        copy_cell(front, 5, i + 4, axe_swing, (i + 3) % 8, 0)

    back = pg.Surface((W * 5, H * 4 * 3)).convert_alpha()
    back.fill((0, 0, 0, 0))

    for y in range(len(SPRITE_MODES)):
        for x in range(len(SPRITE_MODES[y])):
            if SPRITE_MODES[y][x] == 1:
                cut_cell(back, x, y, front, x, y)
            elif SPRITE_MODES[y][x] == 2:
                cut_rect(back, front, (x * W, y * H, W, SPLIT))

    pg.image.save(front, "axe_front.png")
    pg.image.save(back, "axe_back.png")

if __name__ == "__main__":
    main()

#ifndef GUI_H
#define GUI_H

typedef struct world_t world_t;

void gui_init(void);
void gui_load(void);

void gui_update(double fps, int packets, world_t *world);
void gui_render(void);

void gui_toggle_debug();

#endif

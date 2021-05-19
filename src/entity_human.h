#ifndef ENTITY_HUMAN_H
#define ENTITY_HUMAN_H

// TODO IMPLEMENT
typedef struct tool_t tool_t;

struct human_t {
	tool_t *tool;
};
typedef struct human_t human_t;

human_t *human_create(void);
void human_destroy(human_t *);

#endif

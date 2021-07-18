#ifndef L_SYSTEM_H
#define L_SYSTEM_H

typedef struct l_system l_system_t;

l_system_t *l_system_create(const char *axiom);
void l_system_destroy(l_system_t *);

void l_system_add_rule(l_system_t *lsys, const char *symbol, const char *replacement);

// need to free string
const char *l_system_generate(l_system_t *lsys, int iterations);

#endif

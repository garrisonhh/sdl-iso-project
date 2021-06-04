#ifndef MYSTRING_H
#define MYSTRING_H

#include <stddef.h>

typedef struct string_t string_t;

string_t *string_create(const char *initial);
void string_destroy(string_t *);

char *string_get(string_t *);
char *string_get_copy(string_t *);
size_t string_length(string_t *);
char string_index(string_t *, size_t);
void string_set(string_t *, const char *);
void string_append(string_t *, const char *);
void string_replace(string_t *, const char *substring, const char *replacement);

#endif

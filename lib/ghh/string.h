#ifndef GHH_STRING_H
#define GHH_STRING_H

#include <stddef.h>

typedef struct ghh_string string_t;

string_t *string_create(const char *initial);
void string_destroy(string_t *);

// data access
size_t string_length(string_t *);
char *string_raw(string_t *); // returns raw char ptr
char *string_copy(string_t *); // returns copy of raw data

// string ops
void string_set(string_t *, const char *);
void string_append(string_t *, const char *);
void string_replace(string_t *, const char *substring, const char *replacement);

#endif

#ifndef GHH_MEMCHECK_H
#define GHH_MEMCHECK_H

#include <stdlib.h>
#include <stdbool.h>

#ifdef GHH_MEMCHECK_OVERRIDES

// these are only needed in
static inline void *ghh_override_malloc(size_t size) {
    return malloc(size);
}

static inline void *ghh_override_calloc(size_t size, size_t item_size) {
    return calloc(size, item_size);
}

static inline void *ghh_override_realloc(void *ptr, size_t size) {
    return realloc(ptr, size);
}

static inline void ghh_override_free(void *ptr) {
    free(ptr);
}

#endif

#ifndef DEBUG

#define memcheck_init(...)
#define memcheck_quit(...)

#else

void memcheck_init(void);
void memcheck_quit(bool show_total);

void *ghh_alloc(size_t size, const char *file, const int line);
void *ghh_realloc(void *old_ptr, size_t size, const char *file, const int line);
void ghh_free(void *ptr, const char *file, const int line);

#define malloc(size) ghh_alloc(size, __FILE__, __LINE__)
#define calloc(size, item_size) ghh_alloc((size) * (item_size), __FILE__, __LINE__)
#define realloc(ptr, size) ghh_realloc(ptr, size, __FILE__, __LINE__)
#define free(ptr) ghh_free(ptr, __FILE__, __LINE__)

#endif

#endif

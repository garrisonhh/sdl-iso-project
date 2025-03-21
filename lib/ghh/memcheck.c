#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ghh/hashmap.h>
#include <ghh/vector.h>
#include <ghh/utils.h>

#ifdef GHH_MEMCHECK_H
#error "DO NOT INCLUDE <ghh/memcheck.h> IN memcheck.c"
#endif

typedef long long unsigned int nbytes_t;

const nbytes_t KB_BITS = 1024;
const nbytes_t MB_BITS = KB_BITS * KB_BITS;
const nbytes_t GB_BITS = MB_BITS * KB_BITS;

hashmap_t *entries = NULL; // char * => entry_t *
hashmap_t *pointers = NULL; // void * => allocated_t *
nbytes_t current_alloc = 0;
nbytes_t max_alloc = 0;

char SEPARATOR[81];

typedef struct entry {
    size_t current, total;
} entry_t;

typedef struct allocated {
    char *entry_key;
    size_t bytes;
} allocated_t;

typedef struct loggable {
    char **key;
    nbytes_t *bytes;
} loggable_t;

void memcheck_init() {
    entries = hashmap_create(256, -1, true);
    pointers = hashmap_create(256, sizeof(void *), true);

    memset(SEPARATOR, '-', ARRAY_LEN(SEPARATOR));
    SEPARATOR[ARRAY_LEN(SEPARATOR) - 1] = 0;
}

void sprintf_bytes(char *str, int align, nbytes_t bytes) {
    const char *name = " B";
    nbytes_t denom = 0;
    double disp_bytes;

    if (bytes >= GB_BITS) {
        denom = GB_BITS;
        name = "GB";
    } else if (bytes >= MB_BITS) {
        denom = MB_BITS;
        name = "MB";
    } else if (bytes >= KB_BITS) {
        denom = KB_BITS;
        name = "KB";
    }

    if (denom)
        sprintf(str, "%*.2f %s", align - 3, (double)bytes / (double)denom, name);
    else
        sprintf(str, "%*ld %s", align - 3, bytes, name);
}

int loggable_compare(const void *a, const void *b) {
    return *((loggable_t *)b)->bytes - *((loggable_t *)a)->bytes;
}

void print_log(const char *title, char **keys, nbytes_t *bytes, size_t n_items) {
    size_t i;
    char byte_desc[32];
    loggable_t loggables[n_items];

    for (i = 0; i < n_items; ++i)
        loggables[i] = (loggable_t){&keys[i], &bytes[i]};

    qsort(loggables, n_items, sizeof(loggable_t), loggable_compare);

    // print
    puts(SEPARATOR);
    printf("%-75s size\n", title);
    puts(SEPARATOR);

    for (i = 0; i < n_items; ++i) {
        sprintf_bytes(byte_desc, 16, *loggables[i].bytes);
        printf("%-63s %s\n", *loggables[i].key, byte_desc);
    }
}

void memcheck_quit(bool show_total) {
    allocated_t *allocated;
    entry_t *entry;
    hmapiter_t *iter;
    char *key;
    char **keys;
    nbytes_t *bytes;
    char byte_desc[32];
    size_t width;

    iter = hmapiter_create(entries);

    // generate status report
    VECTOR_ALLOC(keys, 0);
    VECTOR_ALLOC(bytes, 0);

    // total allocation
    if (show_total) {
        // generate log
        while (hmapiter_next(iter, (void **)&key, (void **)&entry)) {
            VECTOR_PUSH(keys, key);
            VECTOR_PUSH(bytes, entry->total);
        }

        // print log
        print_log("total", keys, bytes, VECTOR_SIZE(bytes));

        VECTOR_CLEAR(keys);
        VECTOR_CLEAR(bytes);
    }

    // leaked memory
    if (current_alloc) {
        // generate log
        while (hmapiter_next(iter, (void **)&key, (void **)&entry)) {
            if (entry->current) {
                VECTOR_PUSH(keys, key);
                VECTOR_PUSH(bytes, entry->current);
            }
        }

        // print log
        print_log("leaked", keys, bytes, VECTOR_SIZE(bytes));

        sprintf_bytes(byte_desc, 10, current_alloc);
        puts(SEPARATOR);
        printf("%s unfreed found in total.\n", byte_desc);
    } else {
        puts(SEPARATOR);
        printf("all matched memory allocations were properly freed.\n");
    }

    puts(SEPARATOR);

    // cleanup
    VECTOR_FREE(keys);
    VECTOR_FREE(bytes);

    // have to manually free entries and pointer values to prevent segfault on unmatched
    // ghh_free()
    while (hmapiter_next(iter, NULL, (void **)&entry))
        free(entry);

    free(iter);

    HMAP_FOREACH_V(allocated, pointers) {
        free(allocated->entry_key);
        free(allocated);
    }

    hashmap_destroy(entries, false);
    hashmap_destroy(pointers, false);
}

static inline void gen_key(char *str, const char *file, const int line) {
    sprintf(str, "%s:%d", file, line);
}

void log_alloc(void *ptr, size_t size, const char *file, const int line) {
    entry_t *entry;
    allocated_t *allocated;
    char key[100];

    gen_key(key, file, line);

    // map allocated pointer
    allocated = malloc(sizeof(*allocated));

    allocated->entry_key = malloc((strlen(key) + 1) * sizeof(*key));
    allocated->bytes = size;

    strcpy(allocated->entry_key, key);

    hashmap_set(pointers, &ptr, allocated);

    // update proper entry
    if (hashmap_may_get(entries, key, (void **)&entry)) {
        entry->total += size;
        entry->current += size;
    } else {
        entry = malloc(sizeof(*entry));

        entry->current = entry->total = size;

        hashmap_set(entries, key, entry);
    }

    // update global tracking
    current_alloc += size;

    if (current_alloc > max_alloc)
        max_alloc = current_alloc;
}

void log_free(void *ptr, const char *file, const int line) {
    allocated_t *allocated;

    if (hashmap_may_get(pointers, &ptr, (void **)&allocated)) {
        // if segfault from null here, something is broken in pointer mapping
        entry_t *entry = hashmap_get(entries, allocated->entry_key);

        entry->current -= allocated->bytes;
        current_alloc -= allocated->bytes;
    }/* else {
        char unmatched[100];

        gen_key(unmatched, file, line);

        printf("unmatched free at %s\n", unmatched);
    }*/
}

void *ghh_alloc(size_t size, const char *file, const int line) {
    void *ptr = calloc(1, size);

    log_alloc(ptr, size, file, line);

    return ptr;
}

void ghh_free(void *ptr, const char *file, const int line) {
    log_free(ptr, file, line);

    free(ptr);
}

void *ghh_realloc(void *old_ptr, size_t size, const char *file, const int line) {
    void *ptr;

    if (old_ptr != NULL)
        log_free(old_ptr, file, line);

    ptr = realloc(old_ptr, size);

    if (size)
        log_alloc(ptr, size, file, line);

    return ptr;
}

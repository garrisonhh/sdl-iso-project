#ifndef HASH_FUNCTIONS_H
#define HASH_FUNCTIONS_H

#include "hashmap.h"

// for usage with hashmaps

hash_t hash_unsigned(const void *, size_t);
hash_t hash_string(const void *, size_t);
hash_t hash_v3i(const void *, size_t);

#endif

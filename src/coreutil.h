#ifndef CORE_COREUTIL_H
#define CORE_COREUTIL_H

#include <stddef.h>

#define MAX(x,y) ((y) > (x) ? (y) : (x))
#define MIN(x,y) ((y) < (x) ? (y) : (x))

#define container_of(ptr, type, member) \
        ((type *)((char *)(ptr) - offsetof(type, member)))

int needs_grow(size_t minlen, size_t *len);
ptrdiff_t find_index(size_t i, const size_t *base, size_t nel);


#endif /* CORE_COREUTIL_H */

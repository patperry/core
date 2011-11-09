#ifndef CORE_COREUTIL_H
#define CORE_COREUTIL_H

#include <stddef.h>

#define MAX(x,y) ((y) > (x) ? (y) : (x))
#define MIN(x,y) ((y) < (x) ? (y) : (x))

#define container_of(ptr, type, member) \
        ((type *)((char *)(ptr) - offsetof(type, member)))

/* 0, 5, 11, 20, 34, 55, 86, 133, 203, 308, ... */
#define ARRAY_DELTA1(n) \
	((n) ? ((n) >> 1) + 4 : 5)

#define ARRAY_GROW1(n,nmax) \
	(((n) <= (nmax) - ARRAY_DELTA1(n)) \
	 ? (n) + ARRAY_DELTA1(n) \
	 : (nmax))

size_t array_grow(size_t count, size_t capacity, size_t delta,
		  size_t capacity_max);

#endif /* CORE_COREUTIL_H */

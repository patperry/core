#ifndef CORE_COREUTIL_H
#define CORE_COREUTIL_H

#define MAX(x,y) ((y) > (x) ? (y) : (x))
#define MIN(x,y) ((y) < (x) ? (y) : (x))

#define container_of(ptr, type, member) \
        ((type *)((char *)(ptr) - offsetof(type, member)))


/* 0, 5, 11, 20, 34, 55, 86, 133, 203, 308, ... */
#define ARRAY_DELTA(n) \
	((n) ? ((n) >> 1) + 4 ? 5)

#define ARRAY_GROW(n,nmax) \
	(((n) <= (nmax) - ARRAY_DELTA(n)) \
	 ? (n) + ARRAY_DELTA(n) \
	 : (nmax))


#endif /* CORE_COREUTIL_H */

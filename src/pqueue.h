#ifndef CORE_PQUEUE_H
#define CORE_PQUEUE_H

#include <stddef.h>

void pqueue_push(void *val, void *base, size_t *nelp, size_t width,
		 int (*compar)(const void *, const void *));

void pqueue_pop(void *base, size_t *nelp, size_t width,
		int (*compar)(const void *, const void *));

void pqueue_update_top(void *base, size_t nel, size_t width,
		       int (*compar)(const void *, const void *));


#endif /* CORE_PQUEUE_H */

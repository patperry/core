#include <assert.h>
#include <stdint.h>
#include "coreutil.h"

/* 0, 5, 11, 20, 34, 55, 86, 133, 203, 308, ... */
int needs_grow(size_t minlen, size_t *len)
{
	if (minlen <= *len)
		return 0;

	size_t newlen = *len;

	while (newlen < minlen) {
		size_t delta = newlen ? (newlen / 2) + 4 : 5;
		if (newlen <= SIZE_MAX - delta) {
			newlen += delta;
		} else {
			newlen = SIZE_MAX;
		}
	}

	*len = newlen;

	return 1;
}


ptrdiff_t find_index(size_t i, const size_t *base, size_t nel)
{
	const size_t *b = base;
	const size_t *ptr;
	size_t nz;

	for (nz = nel; nz != 0; nz /= 2) {
		ptr = b + (nz / 2);
		if (i == *ptr) {
			return ptr - base;
		}
		if (i > *ptr) {
			b = ptr + 1;
			nz--;
		}
	}

	/* not found */
	return ~((ptrdiff_t)(b - base));
}

#include "xalloc.h"
#include <stdlib.h>

void * xcalloc(size_t count, size_t size)
{
	void *res = calloc(count, size);
	if (size && count && !res) {
		xalloc_die();
	}
	return res;
}

void * xrealloc(void *ptr, size_t size)
{
	void *res = realloc(ptr, size);
	if (!res) {
		xalloc_die();
	}
	return res;
}


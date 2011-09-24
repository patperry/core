/* xalloc.h */

#include <stddef.h>

extern void xalloc_die(void);

void * xcalloc(size_t count, size_t size);
void * xrealloc(void *ptr, size_t size);


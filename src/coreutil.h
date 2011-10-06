#ifndef CORE_COREUTIL_H
#define CORE_COREUTIL_H

#define MAX(x,y) ((y) > (x) ? (y) : (x))
#define MIN(x,y) ((y) < (x) ? (y) : (x))


#define container_of(ptr, type, member) \
        ((type *)((char *)(ptr) - offsetof(type, member)))


#endif /* CORE_COREUTIL_H */

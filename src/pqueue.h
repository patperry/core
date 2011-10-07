#ifndef CORE_PQUEUE_H
#define CORE_PQUEUE_H

#include <stddef.h>

struct pqueue {
	size_t width;
	int (*compar) (const struct pqueue *, const void *, const void *);

	void *base;
	size_t count;
	size_t capacity;
};

/* create, destroy, assign */
void pqueue_init(struct pqueue *q, size_t width,
		 int (*compar)(const struct pqueue*, const void *, const void *));
void pqueue_init_copy(struct pqueue *q, const struct pqueue *src);
void pqueue_assign_copy(struct pqueue *q, const struct pqueue *src);
void pqueue_deinit(struct pqueue *q);

/* properties */
static inline size_t pqueue_count(const struct pqueue *q);
static inline size_t pqueue_width(const struct pqueue *q);
static inline int pqueue_compare(const struct pqueue *q, const void *val1,
				 const void *val2);

static inline size_t pqueue_capacity(const struct pqueue *q);
int pqueue_ensure_capacity(struct pqueue *q, size_t n);

static inline void *pqueue_top(const struct pqueue *q);


/* methods */
void *pqueue_push(struct pqueue *q, const void *val);
void pqueue_pop(struct pqueue *q);
void pqueue_update_top(struct pqueue *q);
void pqueue_clear(struct pqueue *q);
void pqueue_trim_excess(struct pqueue *q);


/* inline funciton definitions */
size_t pqueue_count(const struct pqueue *q)
{
	return q->count;
}

size_t pqueue_width(const struct pqueue *q)
{
	return q->width;
}

int pqueue_compare(const struct pqueue *q, const void *val1, const void *val2)
{
	return q->compar(q, val1, val2);
}

static inline size_t pqueue_capacity(const struct pqueue *q)
{
	return q->capacity;
}

static inline void *pqueue_top(const struct pqueue *q)
{
	return q->base;
}

#endif /* CORE_PQUEUE_H */

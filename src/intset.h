#ifndef CORE_INTSET_H
#define CORE_INTSET_H

#include <stddef.h>

struct intset {
	ptrdiff_t *vals;
	size_t n;
	size_t nmax;
};

/* create, destroy */
void intset_init(struct intset *s);
void intset_init_copy(struct intset *s, const struct intset *src);
void intset_assign_copy(struct intset *s, const struct intset *src);
void intset_deinit(struct intset *s);

/* properties */
static inline size_t intset_count(const struct intset *s);
static inline size_t intset_capacity(const struct intset *s);
static inline void intset_get_vals(const struct intset *s,
				   const ptrdiff_t **vals, size_t *n);

/* methods */
size_t intset_add(struct intset *s, ptrdiff_t val);
void intset_clear(struct intset *s);
int intset_contains(const struct intset *s, ptrdiff_t val);
int intset_remove(struct intset *s, ptrdiff_t val);
void intset_ensure_capacity(struct intset *s, size_t n);
void intset_trim_excess(struct intset *s);

/* index-based operations */
int intset_find(const struct intset *s, ptrdiff_t val, size_t *index);
size_t intset_insert(struct intset *s, size_t index, ptrdiff_t val);
size_t intset_remove_at(struct intset *s, size_t index);

/* inline method definitions */
size_t intset_count(const struct intset *s)
{
	return s->n;
}

size_t intset_capacity(const struct intset *s)
{
	return s->nmax;
}

void intset_get_vals(const struct intset *s, const ptrdiff_t **vals, size_t *n)
{
	*vals = s->vals;
	*n = s->n;
}

#endif /* CORE_INTSET_H */

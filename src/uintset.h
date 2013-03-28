#ifndef CORE_UINTSET_H
#define CORE_UINTSET_H

#include <stddef.h>

struct uintset {
	size_t *vals;
	size_t n;
	size_t nmax;
};

/* create, destroy */
void uintset_init(struct uintset *s);
void uintset_init_copy(struct uintset *s, const struct uintset *src);
void uintset_assign_copy(struct uintset *s, const struct uintset *src);
void uintset_assign_array(struct uintset *s, const size_t *vals, size_t n,
			  int sorted);
void uintset_deinit(struct uintset *s);

/* properties */
static inline size_t uintset_count(const struct uintset *s);
static inline size_t uintset_capacity(const struct uintset *s);
static inline void uintset_get_vals(const struct uintset *s,
				    const size_t **vals, size_t *n);

/* methods */
size_t uintset_add(struct uintset *s, size_t val);
void uintset_clear(struct uintset *s);
int uintset_contains(const struct uintset *s, size_t val);
int uintset_remove(struct uintset *s, size_t val);
void uintset_ensure_capacity(struct uintset *s, size_t n);
void uintset_trim_excess(struct uintset *s);

/* index-based operations */
int uintset_find(const struct uintset *s, size_t val, size_t *index);
size_t uintset_insert(struct uintset *s, size_t index, size_t val);
size_t uintset_remove_at(struct uintset *s, size_t index);

/* inline method definitions */
size_t uintset_count(const struct uintset *s)
{
	return s->n;
}

size_t uintset_capacity(const struct uintset *s)
{
	return s->nmax;
}

void uintset_get_vals(const struct uintset *s, const size_t **vals, size_t *n)
{
	*vals = s->vals;
	*n = s->n;
}

#endif /* CORE_UINTSET_H */

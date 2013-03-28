#include <assert.h>		// assert
#include <stddef.h>		// ptrdiff_t, size_t, NULL
#include <stdlib.h>		// free, qsort
#include <string.h>		// memcpy, memmove
#include "coreutil.h"		// needs_grow
#include "xalloc.h"		// xrealloc

#include "intset.h"

static int compare(const void *px, const void *py)
{
	ptrdiff_t x = *(ptrdiff_t *)px;
	ptrdiff_t y = *(ptrdiff_t *)py;

	if (x > y)
		return +1;
	if (x < y)
		return -1;
	return 0;
}

void intset_init(struct intset *s)
{
	s->vals = NULL;
	s->n = 0;
	s->nmax = 0;
}

void intset_init_copy(struct intset *s, const struct intset *src)
{
	const ptrdiff_t *vals;
	size_t n;

	intset_get_vals(src, &vals, &n);
	s->vals = xmalloc(n * sizeof(ptrdiff_t));
	s->n = n;
	s->nmax = n;
	memcpy(s->vals, vals, n * sizeof(ptrdiff_t));
}

void intset_assign_copy(struct intset *s, const struct intset *src)
{
	const ptrdiff_t *vals;
	size_t n;

	intset_get_vals(src, &vals, &n);
	intset_assign_array(s, vals, n, 1);
}

void intset_assign_array(struct intset *s, const ptrdiff_t *vals, size_t n,
			 int sorted)
{
	intset_ensure_capacity(s, n);
	memcpy(s->vals, vals, n * sizeof(ptrdiff_t));
	s->n = n;

	if (!sorted) {
		qsort(s->vals, s->n, sizeof(ptrdiff_t), compare);
	}
}

void intset_deinit(struct intset *s)
{
	free(s->vals);
}

size_t intset_add(struct intset *s, ptrdiff_t val)
{
	size_t index;

	if (!intset_find(s, val, &index)) {
		intset_insert(s, index, val);
	}

	return index;
}

void intset_clear(struct intset *s)
{
	s->n = 0;
}

int intset_contains(const struct intset *s, ptrdiff_t val)
{
	size_t index;
	int exists;

	exists = intset_find(s, val, &index);

	return exists;
}

int intset_remove(struct intset *s, ptrdiff_t val)
{
	size_t index;
	int exists;

	if ((exists = intset_find(s, val, &index))) {
		intset_remove_at(s, index);
	}

	return exists;
}

void intset_ensure_capacity(struct intset *s, size_t n)
{
	if (needs_grow(n, &s->nmax)) {
		s->vals = xrealloc(s->vals, s->nmax * sizeof(ptrdiff_t));
	}
}

void intset_trim_excess(struct intset *s)
{
	if (s->n != s->nmax) {
		s->nmax = s->n;
		s->vals = xrealloc(s->vals, s->nmax * sizeof(ptrdiff_t));
	}
}

int intset_find(const struct intset *s, ptrdiff_t val, size_t *index)
{
	size_t nel = s->n;
	const ptrdiff_t *base = s->vals;
	const ptrdiff_t *b = base;
	const ptrdiff_t *ptr;
	size_t nz;

	for (nz = nel; nz != 0; nz /= 2) {
		ptr = b + (nz / 2);
		if (val == *ptr) {
			*index = ptr - base;
			return 1;
		}
		if (val > *ptr) {
			b = ptr + 1;
			nz--;
		}
	}

	/* not found */
	*index = b - base;
	return 0;
}

size_t intset_insert(struct intset *s, size_t index, ptrdiff_t val)
{
	size_t n, n1, ntail;

	assert(index <= s->n);
	assert(index == 0 || s->vals[index - 1] < val);
	assert(index == s->n || val < s->vals[index]);

	n = s->n;
	n1 = n + 1;
	ntail = n - index;

	intset_ensure_capacity(s, n1);
	memmove(s->vals + index + 1, s->vals + index,
		ntail * sizeof(ptrdiff_t));
	s->vals[index] = val;
	s->n = n1;

	return ntail;
}

size_t intset_remove_at(struct intset *s, size_t index)
{
	size_t n, n1, ntail;

	assert(index < s->n);

	n = s->n;
	n1 = n - 1;
	ntail = n1 - index;

	memmove(s->vals + index, s->vals + index + 1,
		ntail * sizeof(ptrdiff_t));
	s->n = n1;

	return ntail;
}

#include <assert.h>		// assert
#include <stddef.h>		// size_t, NULL
#include <stdlib.h>		// free, qsort
#include <string.h>		// memcpy, memmove
#include "coreutil.h"		// needs_grow
#include "xalloc.h"		// xrealloc

#include "uintset.h"

static int compare(const void *px, const void *py)
{
	size_t x = *(size_t *)px;
	size_t y = *(size_t *)py;

	if (x > y)
		return +1;
	if (x < y)
		return -1;
	return 0;
}

void uintset_init(struct uintset *s)
{
	s->vals = NULL;
	s->n = 0;
	s->nmax = 0;
}

void uintset_init_copy(struct uintset *s, const struct uintset *src)
{
	const size_t *vals;
	size_t n;

	uintset_get_vals(src, &vals, &n);
	s->vals = xmalloc(n * sizeof(size_t));
	s->n = n;
	s->nmax = n;
	memcpy(s->vals, vals, n * sizeof(size_t));
}

void uintset_assign_copy(struct uintset *s, const struct uintset *src)
{
	const size_t *vals;
	size_t n;

	uintset_get_vals(src, &vals, &n);
	uintset_assign_array(s, vals, n, 1);
}

void uintset_assign_array(struct uintset *s, const size_t *vals, size_t n,
			  int sorted)
{
	uintset_ensure_capacity(s, n);
	memcpy(s->vals, vals, n * sizeof(size_t));
	s->n = n;

	if (!sorted) {
		qsort(s->vals, s->n, sizeof(size_t), compare);
	}
}

void uintset_deinit(struct uintset *s)
{
	free(s->vals);
}

size_t uintset_add(struct uintset *s, size_t val)
{
	size_t index;

	if (!uintset_find(s, val, &index)) {
		uintset_insert(s, index, val);
	}

	return index;
}

void uintset_clear(struct uintset *s)
{
	s->n = 0;
}

int uintset_contains(const struct uintset *s, size_t val)
{
	size_t index;
	int exists;

	exists = uintset_find(s, val, &index);

	return exists;
}

int uintset_remove(struct uintset *s, size_t val)
{
	size_t index;
	int exists;

	if ((exists = uintset_find(s, val, &index))) {
		uintset_remove_at(s, index);
	}

	return exists;
}

void uintset_ensure_capacity(struct uintset *s, size_t n)
{
	if (needs_grow(n, &s->nmax)) {
		s->vals = xrealloc(s->vals, s->nmax * sizeof(size_t));
	}
}

void uintset_trim_excess(struct uintset *s)
{
	if (s->n != s->nmax) {
		s->nmax = s->n;
		s->vals = xrealloc(s->vals, s->nmax * sizeof(size_t));
	}
}

int uintset_find(const struct uintset *s, size_t val, size_t *index)
{
	size_t nel = s->n;
	const size_t *base = s->vals;
	const size_t *b = base;
	const size_t *ptr;
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

size_t uintset_insert(struct uintset *s, size_t index, size_t val)
{
	size_t n, n1, ntail;

	assert(index <= s->n);
	assert(index == 0 || s->vals[index - 1] < val);
	assert(index == s->n || val < s->vals[index]);

	n = s->n;
	n1 = n + 1;
	ntail = n - index;

	uintset_ensure_capacity(s, n1);
	memmove(s->vals + index + 1, s->vals + index, ntail * sizeof(size_t));
	s->vals[index] = val;
	s->n = n1;

	return ntail;
}

size_t uintset_remove_at(struct uintset *s, size_t index)
{
	size_t n, n1, ntail;

	assert(index < s->n);

	n = s->n;
	n1 = n - 1;
	ntail = n1 - index;

	memmove(s->vals + index, s->vals + index + 1, ntail * sizeof(size_t));
	s->n = n1;

	return ntail;
}

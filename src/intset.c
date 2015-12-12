//  Copyright 2015 Patrick O. Perry.
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.

#include <assert.h>		// assert
#include <errno.h>		// ENOMEM
#include <stddef.h>		// size_t, NULL
#include <stdint.h>		// int64_t
#include <stdlib.h>		// free, malloc, qsort, realloc
#include <string.h>		// memcpy, memmove
#include "coreutil.h"		// needs_grow

#include "intset.h"

static int compare(const void *px, const void *py)
{
	int64_t x = *(int64_t *)px;
	int64_t y = *(int64_t *)py;

	if (x > y)
		return +1;
	if (x < y)
		return -1;
	return 0;
}

int intset_init(struct intset *s)
{
	s->vals = NULL;
	s->n = 0;
	s->nmax = 0;
	return 0;
}

int intset_init_copy(struct intset *s, const struct intset *src)
{
	const int64_t *vals;
	size_t n;

	intset_get_vals(src, &vals, &n);
	s->vals = malloc(n * sizeof(int64_t));
	if (!s->vals) {
		s->n = 0;
		s->nmax = 0;
		return ENOMEM;
	}

	s->n = n;
	s->nmax = n;
	memcpy(s->vals, vals, n * sizeof(int64_t));
	return 0;
}

int intset_assign_copy(struct intset *s, const struct intset *src)
{
	const int64_t *vals;
	size_t n;

	intset_get_vals(src, &vals, &n);
	return intset_assign_array(s, vals, n, 1);
}

int intset_assign_array(struct intset *s, const int64_t *vals, size_t n,
			int sorted)
{
	int err;

	if ((err = intset_ensure_capacity(s, n)))
		return err;

	memcpy(s->vals, vals, n * sizeof(int64_t));
	s->n = n;

	if (!sorted) {
		qsort(s->vals, s->n, sizeof(int64_t), compare);
	}

	return 0;
}

void intset_destroy(struct intset *s)
{
	free(s->vals);
}

int intset_add(struct intset *s, int64_t val)
{
	size_t index;

	if (!intset_find(s, val, &index)) {
		return intset_insert(s, index, val);
	}

	return 0;
}

int intset_clear(struct intset *s)
{
	s->n = 0;
	return 0;
}

int intset_contains(const struct intset *s, int64_t val)
{
	size_t index;
	return intset_find(s, val, &index);
}

int intset_remove(struct intset *s, int64_t val)
{
	size_t index;
	int exists;

	if ((exists = intset_find(s, val, &index))) {
		intset_remove_at(s, index);
	}

	return exists;
}

int intset_ensure_capacity(struct intset *s, size_t n)
{
	size_t nmax = s->nmax;

	if (needs_grow(n, &nmax)) {
		int64_t *vals = realloc(s->vals, nmax * sizeof(int64_t));
		if (!vals)
			return ENOMEM;
		s->vals = vals;
		s->nmax = nmax;
	}

	return 0;
}

int intset_trim_excess(struct intset *s)
{
	size_t nmax = s->n;

	if (nmax) {
		int64_t *vals = realloc(s->vals, nmax * sizeof(int64_t));
		if (vals) {
			s->vals = vals;
			s->nmax = nmax;
		}
	} else {
		free(s->vals);
		s->vals = NULL;
		s->nmax = 0;
	}

	return 0;
}

int intset_find(const struct intset *s, int64_t val, size_t *index)
{
	size_t nel = s->n;
	const int64_t *base = s->vals;
	const int64_t *b = base;
	const int64_t *ptr;
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

	// not found
	*index = b - base;
	return 0;
}

int intset_insert(struct intset *s, size_t index, int64_t val)
{
	size_t n, n1, ntail;
	int err;

	assert(index <= s->n);
	assert(index == 0 || s->vals[index - 1] < val);
	assert(index == s->n || val < s->vals[index]);

	n = s->n;
	n1 = n + 1;
	ntail = n - index;

	if ((err = intset_ensure_capacity(s, n1)))
		return err;

	memmove(s->vals + index + 1, s->vals + index,
		ntail * sizeof(int64_t));
	s->vals[index] = val;
	s->n = n1;

	return 0;
}

int intset_remove_at(struct intset *s, size_t index)
{
	size_t n, n1, ntail;

	assert(index < s->n);

	n = s->n;
	n1 = n - 1;
	ntail = n1 - index;

	memmove(s->vals + index, s->vals + index + 1,
		ntail * sizeof(int64_t));
	s->n = n1;

	return 0;
}

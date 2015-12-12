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


#include <alloca.h>
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "coreutil.h"
#include "pqueue.h"

static int array_push(const void *val, void *base, size_t *nelp, size_t width,
		      int (*compar) (const void *, const void *, void *),
		      void *context);

static int array_pop(void *base, size_t *nelp, size_t width,
		     int (*compar) (const void *, const void *, void *),
		     void *context);

static int array_update_top(void *base, size_t nel, size_t width,
			    int (*compar) (const void *, const void *, void *),
			    void *context);

int pqueue_init(struct pqueue *q, size_t width,
		int (*compar) (const void *, const void *, void *),
		void *context)
{
	assert(compar);
	q->width = width;
	q->compar = compar;
	q->context = context;
	q->base = NULL;
	q->count = 0;
	q->capacity = 0;
	return 0;
}

int pqueue_init_copy(struct pqueue *q, const struct pqueue *src)
{
	q->width = src->width;
	q->compar = src->compar;
	q->context = src->context;
	q->base = malloc(src->count * q->width);
	if (!q->base)
		return ENOMEM;

	q->count = src->count;
	q->capacity = src->capacity;
	memcpy(q->base, src->base, q->count * q->width);
	return 0;
}

int pqueue_assign_copy(struct pqueue *q, const struct pqueue *src)
{
	assert(q->width == src->width);
	assert(q->compar == src->compar);
	assert(q->context == src->context);

	if (!pqueue_ensure_capacity(q, src->count))
		return ENOMEM;

	q->count = src->count;
	memcpy(q->base, src->base, q->count * q->width);
	return 0;
}

void pqueue_destroy(struct pqueue *q)
{
	free(q->base);
}

int pqueue_ensure_capacity(struct pqueue *q, size_t n)
{
	if (q->capacity >= n)
		return 0;

	void *base = realloc(q->base, n * q->width);
	if (base) {
		q->capacity = n;
		q->base = base;
		return 0;
	} else {
		return ENOMEM;
	}
}

static int pqueue_grow(struct pqueue *q, size_t delta)
{
	size_t capacity = q->capacity;

	if (needs_grow(q->count + delta, &capacity)) {
		void *base = realloc(q->base, capacity * q->width);
		if (!base)
			return ENOMEM;
		q->base = base;
		q->capacity = capacity;
	}

	return 0;
}

int pqueue_push(struct pqueue *q, const void *val)
{
	int err;

	if (q->count == q->capacity) {
		if ((err = pqueue_grow(q, 1))) {
			return err;
		}
	}

	return array_push(val, q->base, &q->count, q->width, q->compar,
			  q->context);
}

int pqueue_pop(struct pqueue *q)
{
	assert(q->count);
	return array_pop(q->base, &q->count, q->width, q->compar, q->context);
}

int pqueue_update_top(struct pqueue *q)
{
	assert(q->count);
	return array_update_top(q->base, q->count, q->width, q->compar,
				q->context);
}

int pqueue_clear(struct pqueue *q)
{
	q->count = 0;
	return 0;
}

int pqueue_trim_excess(struct pqueue *q)
{
	size_t capacity = q->count;

	if (capacity) {
		void *base = realloc(q->base, capacity * q->width);
		if (base) {
			q->base = base;
			q->capacity = capacity;
		}
	} else {
		free(q->base);
		q->base = NULL;
		q->capacity = 0;
	}

	return 0;
}

int array_push(const void *val, void *base, size_t *nelp, size_t width,
	       int (*compar) (const void *, const void *, void *),
	       void *context)
{
	const size_t nel = *nelp;
	size_t icur = nel;
	void *cur = base + icur * width;

	/* while current element has a parent: */
	while (icur > 0) {
		size_t iparent = (icur - 1) >> 1;
		void *parent = base + iparent * width;

		/* if cur <= parent, heap condition is satisfied */
		if (compar(val, parent, context) <= 0)
			break;

		/* otherwise, swap(cur,parent) */
		memcpy(cur, parent, width);
		icur = iparent;
		cur = parent;
	}

	// actually copy new element
	memcpy(cur, val, width);
	*nelp = nel + 1;
	return 0;
}

int array_pop(void *base, size_t *nelp, size_t width,
	      int (*compar) (const void *, const void *, void *),
	      void *context)
{
	assert(nelp);
	assert(*nelp > 0);

	const size_t nel = *nelp;
	const size_t n = nel - 1;

	if (n == 0)
		goto out;

	/* swap the last element in the tree with the root, then heapify */
	void *val = base + n * width;
	void *cur = base;
	size_t icur = 0;

	// while current element has at least one child
	while ((icur << 1) + 1 < n) {
		size_t ileft = (icur << 1) + 1;
		size_t iright = (icur << 1) + 2;
		size_t imax;
		void *left = base + ileft * width;
		void *right = left + width;
		void *max;

		// find the child with highest priority
		if (iright == n || compar(right, left, context) <= 0) {
			imax = ileft;
			max = left;
		} else {
			imax = iright;
			max = right;
		}

		// stop if heap condition is satisfied
		if (compar(max, val, context) <= 0)
			break;

		// otherwise swap current with maximum child
		memcpy(cur, max, width);
		icur = imax;
		cur = max;
	}

	// actually do the copy
	memcpy(cur, val, width);
out:
	*nelp = n;
	return 0;
}

int array_update_top(void *base, size_t nel, size_t width,
		     int (*compar) (const void *, const void *, void *),
		     void *context)
{
	assert(nel);

	/* make a temporary copy of the old top */
	void *val = alloca(width);
	memcpy(val, base, width);
	size_t icur = 0;
	void *cur = base;

	/* while current element has at least one child... */
	while ((icur << 1) + 1 < nel) {
		size_t ileft = (icur << 1) + 1;
		size_t iright = ileft + 1;
		size_t imax;
		void *left = base + ileft * width;
		void *right = left + width;
		void *max;

		/* find the child with highest priority */
		if (iright == nel || compar(right, left, context) <= 0) {
			imax = ileft;
			max = left;
		} else {
			imax = iright;
			max = right;
		}

		/* stop if heap condition is satisfied */
		if (compar(max, val, context) <= 0)
			break;

		/* otherwise swap current with maximum child */
		memcpy(cur, max, width);
		icur = imax;
		cur = max;
	}

	/* actually do the copy */
	memcpy(cur, val, width);
	return 0;
}

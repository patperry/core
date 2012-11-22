#include <alloca.h>
#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "coreutil.h"
#include "xalloc.h"
#include "pqueue.h"

static void *array_push(const void *val, void *base, size_t *nelp, size_t width,
			int (*compar) (const struct pqueue *,
				       const void *,
				       const void *), const struct pqueue *ctx);

static void array_pop(void *base, size_t *nelp, size_t width,
		      int (*compar) (const struct pqueue *,
				     const void *,
				     const void *), const struct pqueue *ctx);

static void array_update_top(void *base, size_t nel, size_t width,
			     int (*compar) (const struct pqueue *,
					    const void *,
					    const void *),
			     const struct pqueue *ctx);

void pqueue_init(struct pqueue *q, size_t width,
		 int (*compar) (const struct pqueue *, const void *,
				const void *))
{
	assert(compar);
	q->width = width;
	q->compar = compar;
	q->base = NULL;
	q->count = 0;
	q->capacity = 0;
}

void pqueue_init_copy(struct pqueue *q, const struct pqueue *src)
{
	q->width = src->width;
	q->compar = src->compar;
	q->base = xmalloc(src->count * q->width);
	q->count = src->count;
	q->capacity = src->capacity;

	memcpy(q->base, src->base, q->count * q->width);
}

void pqueue_assign_copy(struct pqueue *q, const struct pqueue *src)
{
	assert(q->width == src->width);
	assert(q->compar == src->compar);

	if (!pqueue_ensure_capacity(q, src->count))
		xalloc_die();

	q->count = src->count;
	memcpy(q->base, src->base, q->count * q->width);
}

void pqueue_deinit(struct pqueue *q)
{
	free(q->base);
}

int pqueue_ensure_capacity(struct pqueue *q, size_t n)
{
	if (q->capacity >= n)
		return 1;

	void *base = realloc(q->base, n * q->width);
	if (base) {
		q->capacity = n;
		q->base = base;
		return 1;
	} else {
		return 0;
	}
}

static void pqueue_grow(struct pqueue *q, size_t delta)
{
	if (needs_grow(q->count + delta, &q->capacity)) {
		q->base = xrealloc(q->base, q->capacity * q->width);
	}
}

void *pqueue_push(struct pqueue *q, const void *val)
{
	if (q->count == q->capacity)
		pqueue_grow(q, 1);

	return array_push(val, q->base, &q->count, q->width, q->compar, q);
}

void pqueue_pop(struct pqueue *q)
{
	assert(q->count);

	array_pop(q->base, &q->count, q->width, q->compar, q);
}

void pqueue_update_top(struct pqueue *q)
{
	assert(q->count);
	array_update_top(q->base, q->count, q->width, q->compar, q);
}

void pqueue_clear(struct pqueue *q)
{
	q->count = 0;
}

void pqueue_trim_excess(struct pqueue *q)
{
	q->capacity = q->count;
	if (q->capacity) {
		q->base = xrealloc(q->base, q->capacity * q->width);
	} else {
		free(q->base);
		q->base = NULL;
	}
}

void *array_push(const void *val, void *base, size_t *nelp, size_t width,
		 int (*compar) (const struct pqueue *,
				const void *,
				const void *), const struct pqueue *ctx)
{
	const size_t nel = *nelp;
	size_t icur = nel;
	void *cur = base + icur * width;

	/* while current element has a parent: */
	while (icur > 0) {
		size_t iparent = (icur - 1) >> 1;
		void *parent = base + iparent * width;

		/* if cur <= parent, heap condition is satisfied */
		if (compar(ctx, val, parent) <= 0)
			break;

		/* otherwise, swap(cur,parent) */
		memcpy(cur, parent, width);
		icur = iparent;
		cur = parent;
	}

	// actually copy new element
	memcpy(cur, val, width);
	*nelp = nel + 1;
	return cur;
}

void array_pop(void *base, size_t *nelp, size_t width,
	       int (*compar) (const struct pqueue *,
			      const void *,
			      const void *), const struct pqueue *ctx)
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
		if (iright == n || compar(ctx, right, left) <= 0) {
			imax = ileft;
			max = left;
		} else {
			imax = iright;
			max = right;
		}

		// stop if heap condition is satisfied
		if (compar(ctx, max, val) <= 0)
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
}

void array_update_top(void *base, size_t nel, size_t width,
		      int (*compar) (const struct pqueue *,
				     const void *,
				     const void *), const struct pqueue *ctx)
{
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
		if (iright == nel || compar(ctx, right, left) <= 0) {
			imax = ileft;
			max = left;
		} else {
			imax = iright;
			max = right;
		}

		/* stop if heap condition is satisfied */
		if (compar(ctx, max, val) <= 0)
			break;

		/* otherwise swap current with maximum child */
		memcpy(cur, max, width);
		icur = imax;
		cur = max;
	}

	/* actually do the copy */
	memcpy(cur, val, width);
}

#include <alloca.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "pqueue.h"

void pqueue_push(void *val, void *base, size_t *nelp, size_t width,
		 int (*compar)(const void *, const void *))
{
	const size_t nel = *nelp;
	size_t icur = nel;
	void *cur = base + icur * width;

	/* while current element has a parent: */
	while (icur > 0) {
		size_t iparent = (icur - 1) >> 1;
		void *parent = base + iparent * width;

		/* if cur <= parent, heap condition is satisfied */
		if (compar(val, parent) <= 0)
			break;

		/* otherwise, swap(cur,parent) */
		memcpy(cur, parent, width);
		icur = iparent;
		cur = parent;
	}

	// actually copy new element
	memcpy(cur, val, width);
	*nelp = nel + 1;
}


void pqueue_pop(void *base, size_t *nelp, size_t width,
		int (*compar)(const void *, const void *))
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
		if (iright == n || compar(right, left) <= 0) {
			imax = ileft;
			max = left;
		} else {
			imax = iright;
			max = right;
		}

		// stop if heap condition is satisfied
		if (compar(max, val) <= 0)
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

void pqueue_update_top(void *base, size_t nel, size_t width,
		       int (*compar)(const void *, const void *))
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
		if (iright == nel || compar(right, left) <= 0) {
			imax = ileft;
			max = left;
		} else {
			imax = iright;
			max = right;
		}

		/* stop if heap condition is satisfied */
		if (compar(max, val) <= 0)
			break;

		/* otherwise swap current with maximum child */
		memcpy(cur, max, width);
		icur = imax;
		cur = max;
	}

	/* actually do the copy */
	memcpy(cur, val, width);
}

#include <assert.h>    // assert
#include <stdbool.h>   // bool
#include <stddef.h>    // size_t, NULL
#include <stdint.h>    // uint8_t, uint16_t
#include <stdlib.h>    // free
#include <string.h>    // memset, memmove, memcpy
#include <sys/types.h> // ssize_t
#include "xalloc.h"    // xrealloc
#include "sparsegroup.h"

static ssize_t charbit(ssize_t i)
{
	return i >> 3;
}

static ssize_t modbit(ssize_t i)
{
	return 1 << (i & 7);
}

// We need a small function that tells us how many set bits there are
// in positions 0..i-1 of the bitmap.  It uses a big table.
// We make it static so templates don't allocate lots of these tables.
// There are lots of ways to do this calculation (called 'popcount').
// The 8-bit table lookup is one of the fastest, though this
// implementation suffers from not doing any loop unrolling.  See, eg,
//   http://www.dalkescientific.com/writings/diary/archive/2008/07/03/hakmem_and_other_popcounts.html
//   http://gurmeetsingh.wordpress.com/2008/08/05/fast-bit-counting-routines/
static ssize_t index_to_offset(const uint8_t *bm, ssize_t index)
{
	// We could make these ints.  The tradeoff is size (eg does it overwhelm
	// the cache?) vs efficiency in referencing sub-word-sized array elements
	static const uint8_t bits_in[256] = {	// # of bits set in one char
		0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
		1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
		1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
		2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
		1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
		2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
		2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
		3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
		1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
		2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
		2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
		3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
		2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
		3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
		3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
		4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8,
	};
	ssize_t retval = 0;

	// [Note: condition index > 8 is an optimization; convince yourself we
	// give exactly the same result as if we had index >= 8 here instead.]
	for (; index > 8; index -= 8)	// bm[0..index/8-1]
		retval += bits_in[*bm++];	// chars we want *all* bits in
	return retval + bits_in[*bm & ((1 << index) - 1)];	// the char that includes index
}

static ssize_t offset_to_index(const uint8_t *bm, ssize_t offset)
{
#ifndef NDEBUG
	const uint8_t *const bm0 = bm;
	const ssize_t offset0 = offset;
#endif

	static const uint8_t bits_in[256] = {	// # of bits set in one char
		0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
		1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
		1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
		2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
		1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
		2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
		2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
		3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
		1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
		2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
		2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
		3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
		2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
		3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
		3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
		4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8,
	};
	ssize_t index = 0;
	uint8_t rem;

	while (bits_in[*bm] <= offset) {
		offset -= bits_in[*bm++];
		index += 8;
	}

	if (offset == 0)
		goto done;

	rem = *bm;
	if (bits_in[rem & 0x0F] <= offset) {
		offset -= bits_in[rem & 0x0F];
		index += 4;
		rem >>= 4;
	}
	if (bits_in[rem & 0x03] <= offset) {
		offset -= bits_in[rem & 0x03];
		index += 2;
		rem >>= 2;
	}
	if ((rem & 0x01) <= offset) {
		offset -= rem & 0x01;
		index += 1;
		rem >>= 1;
	}
done:
	assert(offset == 0);
	assert(index_to_offset(bm0, index) == offset0);

	return index;
}


/* bitmap manipulation */
int sparsegroup_bmtest(const struct sparsegroup *g, ssize_t i)
{
	return g->bitmap[charbit(i)] & modbit(i);
}

void sparsegroup_bmset(struct sparsegroup *g, ssize_t i)
{
	g->bitmap[charbit(i)] |= modbit(i);
}

void sparsegroup_bmclear(struct sparsegroup *g, ssize_t i)
{
	g->bitmap[charbit(i)] &= ~modbit(i);
}

int sparsegroup_dtest(const struct sparsegroup *g, ssize_t i)
{
	return g->deleted[charbit(i)] & modbit(i);
}

void sparsegroup_dset(struct sparsegroup *g, ssize_t i)
{
	g->deleted[charbit(i)] |= modbit(i);
}

void sparsegroup_dclear(struct sparsegroup *g, ssize_t i)
{
	g->deleted[charbit(i)] &= ~modbit(i);
}

/* group alloc/free */
void sparsegroup_realloc_group(struct sparsegroup *g, ssize_t n,
			       size_t elt_size)
{
	g->group = xrealloc(g->group, n * elt_size);
}

void sparsegroup_free_group(struct sparsegroup *g, size_t elt_size)
{
	(void)elt_size; // unused
	free(g->group);
	g->group = NULL;
}

/* indexing */
ssize_t sparsegroup_index_to_offset(const struct sparsegroup *g, ssize_t index)
{
	return index_to_offset(g->bitmap, index);
}

ssize_t sparsegroup_offset_to_index(const struct sparsegroup *g, ssize_t offset)
{
	return offset_to_index(g->bitmap, offset);
}

/* constructors */
void sparsegroup_init(struct sparsegroup *g)
{
	g->group = NULL;
	g->num_buckets = 0;
	memset(g->bitmap, 0, sizeof(g->bitmap));
	memset(g->deleted, 0, sizeof(g->deleted));
}

void sparsegroup_deinit(struct sparsegroup *g, size_t elt_size)
{
	sparsegroup_free_group(g, elt_size);
}

/* assign, clear */
void sparsegroup_assign_copy(struct sparsegroup *g,
			     const struct sparsegroup *src, size_t elt_size)
{
	if (g == src)
		return;
	if (src->num_buckets == 0) {
		sparsegroup_free_group(g, elt_size);
	} else {
		sparsegroup_realloc_group(g, src->num_buckets, elt_size);
		memcpy(g->group, src->group, sizeof(g->group));
	}

	memcpy(g->bitmap, src->bitmap, sizeof(g->bitmap));
	memcpy(g->deleted, src->deleted, sizeof(g->deleted));
	g->num_buckets = src->num_buckets;
}

void sparsegroup_clear(struct sparsegroup *g, size_t elt_size)
{
	sparsegroup_free_group(g, elt_size);
	memset(g->bitmap, 0, sizeof(g->bitmap));
	memset(g->deleted, 0, sizeof(g->deleted));
	g->num_buckets = 0;
}

void sparsegroup_clear_tail(struct sparsegroup *g, ssize_t i, size_t elt_size)
{
	assert(0 <= i && i <= sparsegroup_size(g));

	ssize_t n = sparsegroup_size(g);
	ssize_t offset = sparsegroup_index_to_offset(g, i);

	for (; i < n; i++) {
		sparsegroup_bmclear(g, i);
		sparsegroup_dclear(g, i);
	}

	sparsegroup_realloc_group(g, offset, elt_size);
	g->num_buckets = offset;
}

/* informative */
ssize_t sparsegroup_count(const struct sparsegroup *g)
{
	return g->num_buckets;
}

ssize_t sparsegroup_size(const struct sparsegroup *g)
{
	(void)g; // unused;
	return SPARSETABLE_GROUP_SIZE;
}

/* position-based interface */
void *sparsegroup_find(const struct sparsegroup *g, ssize_t index,
		       struct sparsegroup_pos *pos, size_t elt_size)
{
	pos->index = index;
	pos->offset = sparsegroup_index_to_offset(g, index);

	if (sparsegroup_bmtest(g, index)) {
		return (char *)g->group + pos->offset * elt_size;
	}
	return NULL;
}

void *sparsegroup_insert(struct sparsegroup *g,
			 const struct sparsegroup_pos *pos,
			 const void *val, size_t elt_size)
{
	assert(!sparsegroup_bmtest(g, pos->index));

	sparsegroup_realloc_group(g, g->num_buckets + 1, elt_size);

	memmove((char *)g->group + (pos->offset + 1) * elt_size,
		(char *)g->group + pos->offset * elt_size,
		(g->num_buckets - pos->offset) * elt_size);
	g->num_buckets++;
	sparsegroup_bmset(g, pos->index);

	void *res = (char *)g->group + pos->offset * elt_size;

	if (val) {
		memcpy(res, val, elt_size);
	} else {
		memset(res, 0, elt_size);
	}

	return res;
}

void sparsegroup_remove_at(struct sparsegroup *g,
			   const struct sparsegroup_pos *pos, size_t elt_size)
{
	assert(sparsegroup_bmtest(g, pos->index));

	if (g->num_buckets == 1) {
		sparsegroup_free_group(g, elt_size);
	} else {
		memmove((char *)g->group + pos->offset * elt_size,
			(char *)g->group + (pos->offset + 1) * elt_size,
			(g->num_buckets - pos->offset - 1) * elt_size);
		sparsegroup_realloc_group(g, g->num_buckets - 1, elt_size);
	}
	g->num_buckets--;
	sparsegroup_bmclear(g, pos->index);
	sparsegroup_dset(g, pos->index);
}

// true if the index has ever been deleted
bool sparsegroup_deleted(const struct sparsegroup *g,
			 const struct sparsegroup_pos *pos)
{
	return sparsegroup_dtest(g, pos->index);
}

/* iteration */
struct sparsegroup_iter sparsegroup_iter_make(const struct sparsegroup
						     *g, size_t elt_size)
{
	struct sparsegroup_iter it;
	sparsegroup_iter_reset(g, &it, elt_size);
	return it;
}

void sparsegroup_iter_reset(const struct sparsegroup *g,
				   struct sparsegroup_iter *it, size_t elt_size)
{
	it->val = (char *)g->group - elt_size;
	it->pos.index = -1;
	it->pos.offset = -1;
}

bool sparsegroup_iter_advance(const struct sparsegroup *g,
				     struct sparsegroup_iter *it,
				     size_t elt_size)
{
	it->pos.offset++;
	if (it->pos.offset < sparsegroup_count(g)) {
		it->pos.index = sparsegroup_offset_to_index(g, it->pos.offset);
		it->val = (char *)it->val + elt_size;
		assert(it->val == (char *)g->group + it->pos.offset * elt_size);
		return true;
	} else {
		it->pos.index = sparsegroup_size(g);
		it->val = NULL;
		return false;
	}
}



#include <assert.h>
#include <assert.h>    // assert
#include <stdbool.h>   // bool
#include <stddef.h>    // size_t, NULL
#include <stdint.h>    // uint8_t, uint16_t
#include <stdlib.h>    // free
#include <string.h>    // memset, memmove, memcpy
#include <sys/types.h> // ssize_t
#include "xalloc.h"    // xrealloc

#include "sparsegroup.h"
#include "sparsetable.h"
#include "hashset.h"

#define SSIZE_MAX (SIZE_MAX / 2)

/* The probing method */
/* #define JUMP_(key, num_probes) (1)          // Linear probing */
#define JUMP_(key, num_probes)    (num_probes)	// Quadratic probing

/* How full we let the table get before we resize, by default.
 * Knuth says .8 is good -- higher causes us to probe too much,
 * though it saves memory.
 */
#define HT_OCCUPANCY_PCT 80	// (out of 100);

/* Minimum size we're willing to let hashtables be.
 * Must be a power of two, and at least 4.
 * Note, however, that for a given hashtable, the initial size is a
 * function of the first constructor arg, and may be >HT_MIN_BUCKETS.
 */
#define HT_MIN_BUCKETS	4

/* By default, if you don't specify a hashtable size at
 * construction-time, we use this size.  Must be a power of two, and
 * at least HT_MIN_BUCKETS.
 */
#define HT_DEFAULT_STARTING_BUCKETS	32

#define HT_ENLARGE_FACTOR	(HT_OCCUPANCY_PCT / 100.0)

/* The number of buckets must be a power of 2.  This is the largest 
 * power of 2 that a ssize_t can hold.
 */
#define HT_MAX_BUCKETS	((ssize_t)(((size_t)SSIZE_MAX + 1) >> 1))
#define HT_MAX_SIZE	((ssize_t)(HT_MAX_BUCKETS * HT_ENLARGE_FACTOR))

/* This is the smallest size a hashtable can be without being too crowded
 * If you like, you can give a min #buckets as well as a min #elts */
static ssize_t min_buckets(ssize_t num_elts, ssize_t min_buckets_wanted)
{
	assert(0 <= num_elts && num_elts <= HT_MAX_SIZE);
	assert(0 <= min_buckets_wanted && min_buckets_wanted <= HT_MAX_BUCKETS);

	double enlarge = HT_ENLARGE_FACTOR;
	ssize_t sz = HT_MIN_BUCKETS;	// min buckets allowed

	while (sz < min_buckets_wanted || num_elts > (ssize_t)(sz * enlarge)) {
		assert(sz * 2 > sz);
		sz *= 2;
	}
	return sz;
}

/* Reset the enlarge threshold */
static void hashset_reset_thresholds(struct hashset *s, ssize_t num_buckets)
{
	s->enlarge_threshold = num_buckets * HT_ENLARGE_FACTOR;
}

static ssize_t hashset_bucket_count(const struct hashset *s)
{
	return s->num_buckets;
}

static void hashset_init_sized(struct hashset *s,
			       size_t width,
			       size_t (*hash)(const void *),
			       int (*compar)(const void *, const void *),
			       ssize_t num_buckets)
{
	assert(s);
	assert(hash);
	assert(compar);
	assert(num_buckets >= HT_MIN_BUCKETS);

	s->array = xcalloc(num_buckets, width);
	s->num_buckets = num_buckets;
	s->width = width;
	s->status = xcalloc(num_buckets, sizeof(s->status[0]));
	s->count = 0;
	s->hash = hash;
	s->compar = compar;
	hashset_reset_thresholds(s, num_buckets);
}

static void hashset_init_copy_sized(struct hashset *s,
				    const struct hashset *src,
				    ssize_t num_buckets)
{
	assert(s);
	assert(src);
	assert(s != src);
	assert(num_buckets >= HT_MIN_BUCKETS);

	struct hashset_iter it;
	const void *key;

	hashset_init_sized(s, src->width, src->hash, src->compar, num_buckets);

	HASHSET_FOREACH(it, src) {
		key = HASHSET_KEY(it);
		hashset_add(s, key);
	}
}

static bool hashset_needs_grow_delta(const struct hashset *s, ssize_t delta)
{
	assert(delta >= 0);
	assert(s->num_buckets <= HT_MAX_SIZE - delta);

	if (hashset_bucket_count(s) >= HT_MIN_BUCKETS
	    && hashset_count(s) + delta <= s->enlarge_threshold) {
		return false;
	} else {
		return true;
	}
}

static void hashset_grow_delta(struct hashset *s, ssize_t delta)
{
	ssize_t num_nonempty = hashset_count(s);
	ssize_t bucket_count = hashset_bucket_count(s);
	ssize_t resize_to = min_buckets(num_nonempty + delta, bucket_count);

	struct hashset copy;
	hashset_init_copy_sized(&copy, s, resize_to);
	hashset_deinit(s);
	*s = copy;
}

void hashset_init(struct hashset *s,
		  size_t width,
		  size_t (*hash)(const void *),
		  int (*compar)(const void *, const void *))
{
	assert(s);
	assert(hash);
	assert(compar);

	ssize_t num_buckets = HT_DEFAULT_STARTING_BUCKETS;
	hashset_init_sized(s, width, hash, compar, num_buckets);
}

void hashset_init_copy(struct hashset *s, const struct hashset *src)
{
	assert(s);
	assert(src);
	assert(s != src);

	ssize_t num_buckets = hashset_bucket_count(src);
	hashset_init_copy_sized(s, src, num_buckets);
}

void hashset_assign_copy(struct hashset *s, const struct hashset *src)
{
	assert(s);
	assert(src);

	hashset_deinit(s);
	hashset_init_copy(s, src);
}

void hashset_deinit(struct hashset *s)
{
	assert(s);

	free(s->status);
	free(s->array);
}

void *hashset_item(const struct hashset *s, const void *key)
{
	assert(s);
	assert(key);

	struct hashset_pos pos;
	return hashset_find(s, key, &pos);
}

void *hashset_set_item(struct hashset *s, const void *key)
{
	assert(s);
	assert(key);

	struct hashset_pos pos;
	void *dst;

	if ((dst = hashset_find(s, key, &pos))) {
		return memcpy(dst, key, s->width);
	} else {
		return hashset_insert(s, &pos, key);
	}
}

void *hashset_add(struct hashset *s, const void *key)
{
	assert(s);
	assert(key);

	struct hashset_pos pos;
	if (hashset_find(s, key, &pos)) {
		return NULL;
	} else {
		return hashset_insert(s, &pos, key);
	}
}

void hashset_clear(struct hashset *s)
{
	assert(s);

	ssize_t n = hashset_bucket_count(s);
	
	memset(s->array, 0, n * s->width);
	memset(s->status, 0, n * sizeof(s->status[0]));
	s->count = 0;
}

bool hashset_contains(const struct hashset *s, const void *key)
{
	assert(s);
	assert(key);

	return hashset_item(s, key);
}

/* MISSING copy_to */
/* INVALID create_set_comparer */
/* MISSING equals */
/* MISSING except_with */
/* INVALID finalize */
/* UNNECESSARY get_enumerator (iter_make) */
/* MISSING get_hash_code */
/* UNNECESSARY get_object_data */
/* INVALID get_type */
/* MISSING intersect_with */
/* MISSING is_proper_subset_of */
/* MISSING is_proper_superset_of */
/* MISSING is_subset_of */
/* MISSING is_superset_of */
/* UNNESSARY memberwise_clone (assign_copy) */
/* INVALID on_deserialization */
/* MISSING overlaps */

bool hashset_remove(struct hashset *s, const void *key)
{
	assert(s);
	assert(key);

	bool found;

	struct hashset_pos pos;
	if ((found = hashset_find(s, key, &pos))) {
		hashset_remove_at(s, &pos);
	}
	assert(!hashset_contains(s, key));
	return found;
}

/* MISSING remove_where */
/* MISSING set_equals */
/* MISSING symmetric_except_with */
/* MISSING to_string */

void hashset_trim_excess(struct hashset *s)
{
	assert(s);

	ssize_t count = hashset_count(s);
	ssize_t resize_to = min_buckets(count, 0);

	struct hashset copy;
	hashset_init_copy_sized(&copy, s, resize_to);
	hashset_deinit(s);
	*s = copy;
}

/* MISSING union_with */

void *hashset_find(const struct hashset *s, const void *key,
		   struct hashset_pos *pos)
{
	assert(s);
	assert(key);
	assert(pos);

	const void *array = s->array;
	const uint8_t *status = s->status;
	const ssize_t bucket_count = s->num_buckets;
	const size_t width = s->width;
	ssize_t num_probes = 0;	// how many times we've probed
	const ssize_t bucket_count_minus_one = bucket_count - 1;
	size_t hash = hashset_hash(s, key);
	ssize_t bucknum = hash & bucket_count_minus_one;
	void *ptr;
	bool full, deleted;

	pos->hash = hash;
	pos->has_insert = false;
	pos->has_existing = false;

	for (num_probes = 0; num_probes < bucket_count; num_probes++) {
		ptr = (char *)array + bucknum * width;
		full = status[bucknum] & HASHSET_BIN_FULL;
		deleted = status[bucknum] & HASHSET_BIN_DELETED;
		if (!full && !deleted) {	// bucket is empty
			if (!pos->has_insert) {	// found no prior place to insert
				pos->insert = bucknum;
				pos->has_insert = true;
			}
			pos->has_existing = false;
			return NULL;
		} else if (!full && deleted) {	// keep searching, but mark to insert
			if (!pos->has_insert) {
				pos->insert = bucknum;
				pos->has_insert = true;
			}
		} else if (!hashset_compare(s, key, ptr)) {
			pos->existing = bucknum;
			pos->has_existing = true;
			return ptr;
		}
		bucknum =
		    (bucknum +
		     JUMP_(key, num_probes + 1)) & bucket_count_minus_one;
	}

	return NULL;		// table is full and key is not present
}

void *hashset_insert(struct hashset *s, struct hashset_pos *pos,
		     const void *key)
{
	assert(s);
	assert(pos);
	assert(!pos->has_existing);
	assert(key);
	assert(hashset_hash(s, key) == pos->hash);

	if (hashset_needs_grow_delta(s, 1)) {
		hashset_grow_delta(s, 1);
		hashset_find(s, key, pos);	// need to recompute pos
	}

	assert(pos->has_insert);

	ssize_t ix = pos->insert;
	ssize_t width = s->width;
	
	s->count++;
	s->status[ix] |= HASHSET_BIN_FULL;
	
	void *ptr = s->array + ix * width;
	if (key) {
		memcpy(ptr, key, width);
	}
	
	return ptr;
}

void hashset_remove_at(struct hashset *s, struct hashset_pos *pos)
{
	assert(s);
	assert(pos);
	assert(pos->has_existing);

	ssize_t ix = pos->existing;
	size_t width = s->width;
	s->count--;
	memset(s->array + ix * width, 0, width);
	s->status[ix] = HASHSET_BIN_DELETED;
}

struct hashset_iter hashset_iter_make(const struct hashset *s)
{
	assert(s);

	struct hashset_iter it;
	it.s = s;
	hashset_iter_reset(&it);
	return it;
}

void hashset_iter_reset(struct hashset_iter *it)
{
	assert(it);

	it->i = 0;
	it->val = NULL;
}

void *hashset_iter_advance(struct hashset_iter *it)
{
	assert(it);

	const struct hashset *s = it->s;
	const uint8_t *status = it->s->status;
	ssize_t i, n = hashset_bucket_count(s);
	
	for (i = it->i; i < n; i++) {
		if (status[i] & HASHSET_BIN_FULL) {
			it->val = s->array + i * s->width;
			goto out;
		}
	}
	it->val = NULL;
out:
	it->i = i + 1;
	return it->val;
}

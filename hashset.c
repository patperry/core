#include <assert.h>    // assert
#include <limits.h>    // CHAR_BIT
#include <stdbool.h>   // bool
#include <stddef.h>    // size_t, NULL
#include <stdint.h>    // uint8_t, uint16_t
#include <stdlib.h>    // free
#include <string.h>    // memset, memmove, memcpy
#include "xalloc.h"    // xrealloc

#include "hashset.h"

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

/* The number of buckets must be a power of 2.  This is the largest 
 * power of 2 that a size_t can hold.
 */
#define HT_MAX_BUCKETS	((size_t)1 << (CHAR_BIT * sizeof(size_t) - 1))
#define HT_MAX_COUNT	(HT_OCCUPANCY_PCT * (HT_MAX_BUCKETS / 100))

/* The following is more accurate than ((pct)/100.0 * (x)) when x is
 * really big (> 2^52).
 */
#define PERCENT(pct,x) \
	((pct) * ((x) / 100) \
	 + (size_t)((pct) * (((x) % 100) / 100.0)))

/* This is the smallest size a hashtable can be without being too crowded
 * If you like, you can give a min #buckets as well as a min #elts */
static size_t min_buckets(size_t count, size_t nbucket0)
{
	assert(count <= HT_MAX_COUNT);
	assert(nbucket0 <= HT_MAX_BUCKETS);

	size_t n = HT_MIN_BUCKETS;	// min buckets allowed
	
	while (n < nbucket0 || count > PERCENT(HT_OCCUPANCY_PCT, n)) {
		assert(n < SIZE_MAX / 2);
		n *= 2;
	}

	assert(n >= nbucket0);
	assert(count <= PERCENT(HT_OCCUPANCY_PCT, n));
	
	return n;
}

/* Reset the enlarge threshold */
static void hashset_reset_thresholds(struct hashset *s, size_t nbucket)
{
	s->enlarge_threshold = PERCENT(HT_OCCUPANCY_PCT, nbucket);
}

static size_t hashset_bucket_count(const struct hashset *s)
{
	return s->nbucket;
}

static void hashset_init_sized(struct hashset *s,
			       size_t width,
			       size_t (*hash)(const void *),
			       int (*compar)(const void *, const void *),
			       size_t nbucket)
{
	assert(s);
	assert(hash);
	assert(compar);
	assert(nbucket >= HT_MIN_BUCKETS);

	s->buckets = xcalloc(nbucket, width);
	s->nbucket = nbucket;
	s->width = width;
	s->status = xcalloc(nbucket, sizeof(s->status[0]));
	s->count = 0;
	s->hash = hash;
	s->compar = compar;
	hashset_reset_thresholds(s, nbucket);
}

static void hashset_init_copy_sized(struct hashset *s,
				    const struct hashset *src,
				    size_t nbucket)
{
	assert(s);
	assert(src);
	assert(s != src);
	assert(nbucket >= HT_MIN_BUCKETS);

	struct hashset_iter it;
	const void *key;

	hashset_init_sized(s, src->width, src->hash, src->compar, nbucket);

	HASHSET_FOREACH(it, src) {
		key = HASHSET_KEY(it);
		hashset_add(s, key);
	}
}

static bool hashset_needs_grow_delta(const struct hashset *s, size_t delta)
{
	assert(delta <= HT_MAX_COUNT);
	assert(s->nbucket <= HT_MAX_COUNT - delta);
	assert(s->enlarge_threshold >= s->count);
	
	if (s->nbucket >= HT_MIN_BUCKETS
	    && delta <= s->enlarge_threshold - s->count) {
		return false;
	} else {
		return true;
	}
}

static void hashset_grow_delta(struct hashset *s, size_t delta)
{
	assert(delta <=  HT_MAX_COUNT - s->nbucket);

	size_t count0 = s->count;
	size_t nbucket0 = s->nbucket;
	size_t count = count0 + delta;
	size_t nbucket = min_buckets(count, nbucket0);

	struct hashset copy;
	hashset_init_copy_sized(&copy, s, nbucket);
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

	size_t nbucket = HT_DEFAULT_STARTING_BUCKETS;
	hashset_init_sized(s, width, hash, compar, nbucket);
}

void hashset_init_copy(struct hashset *s, const struct hashset *src)
{
	assert(s);
	assert(src);
	assert(s != src);

	size_t nbucket = hashset_bucket_count(src);
	hashset_init_copy_sized(s, src, nbucket);
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
	free(s->buckets);
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

	size_t n = hashset_bucket_count(s);
	
	memset(s->buckets, 0, n * s->width);
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

	size_t count = hashset_count(s);
	size_t resize_to = min_buckets(count, 0);

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

	const void *buckets = s->buckets;
	const uint8_t *status = s->status;
	const size_t bucket_count = s->nbucket;
	const size_t width = s->width;
	size_t num_probes = 0;	// how many times we've probed
	const size_t bucket_count_minus_one = bucket_count - 1;
	size_t hash = hashset_hash(s, key);
	size_t bucknum = hash & bucket_count_minus_one;
	void *ptr;
	bool full, deleted;

	pos->hash = hash;
	pos->has_insert = false;
	pos->has_existing = false;

	for (num_probes = 0; num_probes < bucket_count; num_probes++) {
		ptr = (char *)buckets + bucknum * width;
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

	assert(!hashset_needs_grow_delta(s, 1));	
	assert(pos->has_insert);

	size_t ix = pos->insert;
	size_t width = s->width;
	
	s->count++;
	s->status[ix] |= HASHSET_BIN_FULL;
	
	void *ptr = s->buckets + ix * width;
	if (key) {
		memcpy(ptr, key, width);
	}
	
	assert(s->count <= s->enlarge_threshold);
	
	return ptr;
}

void hashset_remove_at(struct hashset *s, struct hashset_pos *pos)
{
	assert(s);
	assert(pos);
	assert(pos->has_existing);

	size_t ix = pos->existing;
	size_t width = s->width;
	s->count--;
	memset(s->buckets + ix * width, 0, width);
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
	size_t i, n = hashset_bucket_count(s);
	
	for (i = it->i; i < n; i++) {
		if (status[i] & HASHSET_BIN_FULL) {
			it->val = s->buckets + i * s->width;
			goto out;
		}
	}
	it->val = NULL;
out:
	it->i = i + 1;
	return it->val;
}

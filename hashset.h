#ifndef _HASHSET_H
#define _HASHSET_H

#include <stddef.h>

struct hashset {
	size_t width;
	size_t (*hash) (const void *);
	int (*compar) (const void *, const void *);

	size_t nbucket;	
	void *buckets;
	unsigned char *status;

	size_t count;
	size_t count_max;
};

struct hashset_pos {
	size_t hash;
	size_t insert;
	size_t existing;
};

struct hashset_iter {
	const struct hashset *s;
	size_t i;
	void *val;
};

#define HASHSET_KEY(it) ((it).val)
#define HASHSET_FOREACH(it, set) \
	for ((it) = hashset_iter_make(set); hashset_iter_advance(&(it));)


/* create, destroy */
void hashset_init(struct hashset *s, size_t width,
		  size_t (*hash)(const void *),
		  int (*compar)(const void *, const void *));
void hashset_init_copy(struct hashset *s, const struct hashset *src);
void hashset_assign_copy(struct hashset *s, const struct hashset *src);
void hashset_deinit(struct hashset *s);

/* properties */
static inline size_t hashset_count(const struct hashset *s);
static inline size_t hashset_width(const struct hashset *s);
static inline int hashset_compare(const struct hashset *s, const void *key1,
				  const void *key2);
static inline size_t hashset_hash(const struct hashset *s, const void *key);

void *hashset_item(const struct hashset *s, const void *key);
void *hashset_set_item(struct hashset *s, const void *key);

/* methods */
void *hashset_add(struct hashset *s, const void *key);
void hashset_clear(struct hashset *s);
int hashset_contains(const struct hashset *s, const void *key);
int hashset_remove(struct hashset *s, const void *key);
void hashset_trim_excess(struct hashset *s);

/* position-based operations */
void *hashset_find(const struct hashset *s, const void *key,
		   struct hashset_pos *pos);
void *hashset_insert(struct hashset *s, struct hashset_pos *pos,
		     const void *key);
void hashset_remove_at(struct hashset *s, struct hashset_pos *pos);

/* iteration */
struct hashset_iter hashset_iter_make(const struct hashset *s);
void hashset_iter_reset(struct hashset_iter *it);
void *hashset_iter_advance(struct hashset_iter *it);

/* static method definitions */
size_t hashset_count(const struct hashset *s)
{
	return s->count;
}

size_t hashset_width(const struct hashset *s)
{
	return s->width;
}

static inline int hashset_compare(const struct hashset *s, const void *key1,
				  const void *key2)
{
	return s->compar(key1, key2);
}

static inline size_t hashset_hash(const struct hashset *s, const void *key)
{
	return s->hash(key);
}

#endif /* _HASHSET_H */

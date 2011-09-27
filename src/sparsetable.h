#ifndef _SPARSETABLE_H
#define _SPARSETABLE_H

/* An intmap maps the integers 0..n-1 to values.  The implementation on
 * Google's "sparsetable".
 *
 * Define SSIZE_MAX, ssize_t, bool and assert before including this file.
 */

#include <stddef.h>		// sizeof, size_t


/* public */
struct sparsetable {
	struct sparsegroup *groups;
	size_t num_groups;
	size_t table_size;	// how many buckets they want
	size_t num_buckets;	// number of non-empty buckets
	size_t elt_size;
};

struct sparsetable_pos {
	ssize_t index;
	struct sparsegroup *group;
	struct sparsegroup_pos group_pos;
};

struct sparsetable_iter {
	const struct sparsetable *table;
	struct sparsegroup *group;
	ssize_t index;
	struct sparsegroup_iter group_it;
};

#define SPARSETABLE_VAL(it) SPARSEGROUP_VAL((it).group_it)
#define SPARSETABLE_IDX(it) SPARSEGROUP_VAL((it).index)
#define SPARSETABLE_FOREACH(it, t) \
	for ((it) = sparsetable_iter_make(t); sparsetable_iter_advance(&(it));)

/* constructors */
void sparsetable_init(struct sparsetable *t, ssize_t n, size_t elt_size);
void sparsetable_init_copy(struct sparsetable *t,
			   const struct sparsetable *src);
void sparsetable_deinit(struct sparsetable *t);

/* assign, copy, clear */
void sparsetable_assign_copy(struct sparsetable *t,
			     const struct sparsetable *src);
void sparsetable_clear(struct sparsetable *t);

/* properties */
ssize_t sparsetable_count(const struct sparsetable *t);
ssize_t sparsetable_size(const struct sparsetable *t);
void sparsetable_set_size(struct sparsetable *t, ssize_t n);
size_t sparsetable_elt_size(const struct sparsetable *t);

/* position-based interface */
void *sparsetable_find(const struct sparsetable *t, ssize_t index,
		       struct sparsetable_pos *pos);
void *sparsetable_insert(struct sparsetable *t,
			 const struct sparsetable_pos *pos, const void *val);
void sparsetable_remove_at(struct sparsetable *t,
			   const struct sparsetable_pos *pos);
bool sparsetable_deleted(const struct sparsetable *t,
			 const struct sparsetable_pos *pos);

/* iteration */
struct sparsetable_iter sparsetable_iter_make(const struct sparsetable *t);
void sparsetable_iter_reset(struct sparsetable_iter *it);
bool sparsetable_iter_advance(struct sparsetable_iter *it);

#endif /* _SPARSETABLE_H */

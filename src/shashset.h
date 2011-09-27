#ifndef SHASHSET_H
#define SHASHSET_H

/* Copyright (c) 2011, Patrick O. Perry
 * Copyright (c) 2005, Google Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following disclaimer
 *       in the documentation and/or other materials provided with the
 *       distribution.
 *     * Neither the names of Patrick O. Perry, Google Inc,. nor the names of
 *       their contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stddef.h>

struct shashset {
	struct sparsetable table;
	uint32_t (*hash) (const void *);
	bool (*equals) (const void *, const void *);
	ssize_t enlarge_threshold;	// (table size) * enlarge_factor
};

struct shashset_pos {
	uint32_t hash;
	struct sparsetable_pos insert;
	struct sparsetable_pos existing;
	bool has_existing;
	bool has_insert;
};

struct shashset_iter {
	struct sparsetable_iter table_it;
};
#define SHASHSET_KEY(it) SPARSETABLE_VAL((it).table_it)
#define SHASHSET_FOREACH(it, set) \
	for ((it) = shashset_iter_make(set); shashset_iter_advance(&(it));)

/* create, destroy */
void shashset_init(struct shashset *s, uint32_t (*hash) (const void *),
		   bool (*equals) (const void *, const void *),
		   size_t elt_size);
void shashset_init_copy(struct shashset *s, const struct shashset *src);
void shashset_assign_copy(struct shashset *s, const struct shashset *src);
void shashset_deinit(struct shashset *s);

/* properties */
static inline ssize_t shashset_count(const struct shashset *s);
static inline size_t shashset_elt_size(const struct shashset *s);
static inline bool shashset_equals(const struct shashset *s, const void *key1,
				   const void *key2);
static inline uint32_t shashset_hash(const struct shashset *s, const void *key);

void *shashset_item(const struct shashset *s, const void *key);
void *shashset_set_item(struct shashset *s, const void *key);

/* methods */
void *shashset_add(struct shashset *s, const void *key);
void shashset_clear(struct shashset *s);
bool shashset_contains(const struct shashset *s, const void *key);
bool shashset_remove(struct shashset *s, const void *key);
void shashset_trim_excess(struct shashset *s);

/* position-based operations */
void *shashset_find(const struct shashset *s, const void *key,
		    struct shashset_pos *pos);
void *shashset_insert(struct shashset *s, struct shashset_pos *pos,
		      const void *key);
void shashset_remove_at(struct shashset *s, struct shashset_pos *pos);

/* iteration */
struct shashset_iter shashset_iter_make(const struct shashset *s);
void shashset_iter_reset(struct shashset_iter *it);
bool shashset_iter_advance(struct shashset_iter *it);

/* static method definitions */
ssize_t shashset_count(const struct shashset *s)
{
	return sparsetable_count(&s->table);
}

size_t shashset_elt_size(const struct shashset *s)
{
	return sparsetable_elt_size(&s->table);
}

static inline bool shashset_equals(const struct shashset *s, const void *key1,
				   const void *key2)
{
	return s->equals(key1, key2);
}

static inline uint32_t shashset_hash(const struct shashset *s, const void *key)
{
	return s->hash(key);
}

#endif /* SHASHSET_H */

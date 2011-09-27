#ifndef SPARSEGROUP_H
#define SPARSEGROUP_H

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

/* private */
#define SPARSETABLE_GROUP_SIZE 48

struct sparsegroup {
	void *group;		// (small) array of values
	uint16_t num_buckets;	// limits GROUP_SIZE to 64K
	uint8_t bitmap[(SPARSETABLE_GROUP_SIZE - 1) / 8 + 1];	// fancy math is so we round up
	uint8_t deleted[(SPARSETABLE_GROUP_SIZE - 1) / 8 + 1];	// indicates if a position was ever deleted
};

struct sparsegroup_pos {
	ssize_t index;
	ssize_t offset;
};

struct sparsegroup_iter {
	void *val;
	struct sparsegroup_pos pos;
};

#define SPARSEGROUP_VAL(it) ((it).val)
#define SPARSEGROUP_IDX(it) ((it).pos.index)

/* bitmap manipulation */
int sparsegroup_bmtest(const struct sparsegroup *g, ssize_t i);
void sparsegroup_bmset(struct sparsegroup *g, ssize_t i);
void sparsegroup_bmclear(struct sparsegroup *g, ssize_t i);
int sparsegroup_dtest(const struct sparsegroup *g, ssize_t i);
void sparsegroup_dset(struct sparsegroup *g, ssize_t i);
void sparsegroup_dclear(struct sparsegroup *g, ssize_t i);

/* group alloc/free */
void sparsegroup_realloc_group(struct sparsegroup *g, ssize_t n,
				      size_t elt_size);
void sparsegroup_free_group(struct sparsegroup *g, size_t elt_size);

/* indexing */
ssize_t sparsegroup_index_to_offset(const struct sparsegroup *g,
					   ssize_t index);
ssize_t sparsegroup_offset_to_index(const struct sparsegroup *g,
					   ssize_t offset);

/* public functions */

/* constructors */
void sparsegroup_init(struct sparsegroup *g);
void sparsegroup_deinit(struct sparsegroup *g, size_t elt_size);

/* assign, clear */
void sparsegroup_assign_copy(struct sparsegroup *g,
				    const struct sparsegroup *src,
				    size_t elt_size);
void sparsegroup_clear(struct sparsegroup *g, size_t elt_size);
void sparsegroup_clear_tail(struct sparsegroup *g, ssize_t i,
				   size_t elt_size);

/* informative */
ssize_t sparsegroup_count(const struct sparsegroup *g);
ssize_t sparsegroup_size(const struct sparsegroup *g);

/* position-based interface */
void *sparsegroup_find(const struct sparsegroup *g, ssize_t index,
			      struct sparsegroup_pos *pos, size_t elt_size);
void *sparsegroup_insert(struct sparsegroup *g,
				const struct sparsegroup_pos *pos,
				const void *val, size_t elt_size);
void sparsegroup_remove_at(struct sparsegroup *g,
				  const struct sparsegroup_pos *pos,
				  size_t elt_size);
bool sparsegroup_deleted(const struct sparsegroup *g,
				const struct sparsegroup_pos *pos);

/* iteration */
struct sparsegroup_iter sparsegroup_iter_make(const struct sparsegroup
						     *g, size_t elt_size);
void sparsegroup_iter_reset(const struct sparsegroup *g,
				   struct sparsegroup_iter *it,
				   size_t elt_size);
bool sparsegroup_iter_advance(const struct sparsegroup *g,
				     struct sparsegroup_iter *it,
				     size_t elt_size);

#endif /* SPARSEGROUP_H */

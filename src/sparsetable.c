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

#include <assert.h>		// assert
#include <stdbool.h>		// bool
#include <stddef.h>		// size_t, NULL
#include <stdint.h>		// uint8_t, uint16_t
#include <stdlib.h>		// free
#include <string.h>		// memset, memmove, memcpy
#include <sys/types.h>		// ssize_t
#include "xalloc.h"		// xrealloc

#include "sparsegroup.h"
#include "sparsetable.h"

////////////////////////////////////////////////////////////////////////////////

// SPARSE-TABLE
// ------------
// The idea is that a table with (logically) t buckets is divided
// into t/M *groups* of M buckets each.  (M is a constant set in
// SPARSETABLE_GROUP_SIZE for efficiency.)  Each group is stored sparsely.
// Thus, inserting into the table causes some array to grow, which is
// slow but still constant time.  Lookup involves doing a
// logical-position-to-sparse-position lookup, which is also slow but
// constant time.  The larger M is, the slower these operations are
// but the less overhead (slightly).
//
// To store the sparse array, we store a bitmap B, where B[i] = 1 iff
// bucket i is non-empty.  Then to look up bucket i we really look up
// array[# of 1s before i in B].  This is constant time for fixed M.
//

/* How to deal with the proper group */
static ssize_t sparsetable_num_groups(ssize_t n);
static uint16_t sparsetable_pos_in_group(ssize_t i);
static ssize_t sparsetable_group_num(ssize_t i);
static struct sparsegroup *sparsetable_which_group(const struct sparsetable *t,
						   ssize_t i);

ssize_t sparsetable_num_groups(ssize_t n)
{				// how many to hold n buckets
	return n == 0 ? 0 : ((n - 1) / SPARSETABLE_GROUP_SIZE) + 1;
}

uint16_t sparsetable_pos_in_group(ssize_t i)
{
	return (uint16_t)(i % SPARSETABLE_GROUP_SIZE);
}

ssize_t sparsetable_group_num(ssize_t i)
{
	return i / SPARSETABLE_GROUP_SIZE;
}

struct sparsegroup *sparsetable_which_group(const struct sparsetable *t,
					    ssize_t i)
{
	struct sparsegroup *groups = t->groups;
	return &groups[sparsetable_group_num(i)];
}

void sparsetable_init(struct sparsetable *t, ssize_t n, size_t elt_size)
{
	t->groups = NULL;
	t->num_groups = 0;
	t->table_size = 0;
	t->num_buckets = 0;
	t->elt_size = elt_size;
	sparsetable_set_size(t, n);
}

void sparsetable_init_copy(struct sparsetable *t, const struct sparsetable *src)
{
	size_t elt_size = src->elt_size;
	ssize_t i, n = src->table_size;
	struct sparsegroup *groups;
	const struct sparsegroup *src_groups;

	sparsetable_init(t, n, elt_size);

	if (n > 0) {
		groups = t->groups;
		src_groups = src->groups;

		for (i = 0; i < n; i++) {
			sparsegroup_assign_copy(groups + i, src_groups + i,
						elt_size);
		}
	}

	t->num_buckets = src->num_buckets;
}

void sparsetable_deinit(struct sparsetable *t)
{
	struct sparsegroup *groups = t->groups;
	ssize_t i, n = t->num_groups;
	size_t elt_size = t->elt_size;

	for (i = 0; i < n; i++) {
		sparsegroup_deinit(groups + i, elt_size);
	}
	free(t->groups);
}

void sparsetable_assign_copy(struct sparsetable *t,
			     const struct sparsetable *src)
{
	sparsetable_deinit(t);
	sparsetable_init_copy(t, src);
}

void sparsetable_clear(struct sparsetable *t)
{
	struct sparsegroup *groups = t->groups;
	ssize_t i, n = t->num_groups;
	size_t elt_size = t->elt_size;

	for (i = 0; i < n; i++) {
		sparsegroup_clear(groups + i, elt_size);
	}

	t->num_buckets = 0;
}

ssize_t sparsetable_count(const struct sparsetable *t)
{
	return t->num_buckets;
}

ssize_t sparsetable_size(const struct sparsetable *t)
{
	return t->table_size;
}

size_t sparsetable_elt_size(const struct sparsetable *t)
{
	return t->elt_size;
}

void sparsetable_set_size(struct sparsetable *t, ssize_t n)
{
	size_t elt_size = sparsetable_elt_size(t);
	ssize_t num_groups0 = t->num_groups;
	ssize_t num_groups = sparsetable_num_groups(n);
	ssize_t i, n0, index;
	struct sparsegroup *g;

	if (num_groups <= num_groups0) {
		for (i = num_groups; i < num_groups0; i++) {
			g = &t->groups[i];
			assert(!sparsegroup_count(g));
			sparsegroup_deinit(g, elt_size);
		}

		t->groups = xrealloc(t->groups,
				     num_groups * sizeof(t->groups[0]));
		t->num_groups = num_groups;

		n0 = t->table_size;
		index = sparsetable_pos_in_group(n);

		if (n < n0 && index > 0) {	// lower num_buckets, clear last group
			g = &t->groups[num_groups - 1];
			assert(sparsegroup_index_to_offset(g, index) ==
			       sparsegroup_count(g));
			sparsegroup_clear_tail(g, index, elt_size);
		}
	} else {
		t->groups = xrealloc(t->groups,
				     num_groups * sizeof(t->groups[0]));
		t->num_groups = num_groups;
		// no need to zero memory since sparsegroup_init never fails

		for (i = num_groups0; i < num_groups; i++) {
			g = &t->groups[i];
			sparsegroup_init(g);
		}
	}

	t->table_size = n;
}

void *sparsetable_find(const struct sparsetable *t, ssize_t index,
		       struct sparsetable_pos *pos)
{
	ssize_t index_in_group = sparsetable_pos_in_group(index);
	size_t elt_size = sparsetable_elt_size(t);

	pos->index = index;
	pos->group = sparsetable_which_group(t, index);
	return sparsegroup_find(pos->group, index_in_group, &pos->group_pos,
				elt_size);
}

void *sparsetable_insert(struct sparsetable *t,
			 const struct sparsetable_pos *pos, const void *val)
{
	void *res;
	if ((res = sparsegroup_insert(pos->group, &pos->group_pos, val,
				      sparsetable_elt_size(t)))) {
		t->num_buckets++;
	}
	return res;
}

void sparsetable_remove_at(struct sparsetable *t,
			   const struct sparsetable_pos *pos)
{
	sparsegroup_remove_at(pos->group, &pos->group_pos,
			      sparsetable_elt_size(t));
	t->num_buckets--;
}

bool sparsetable_deleted(const struct sparsetable *t,
			 const struct sparsetable_pos *pos)
{
	(void)t;		// unused
	return sparsegroup_deleted(pos->group, &pos->group_pos);
}

/* iteration */
struct sparsetable_iter sparsetable_iter_make(const struct sparsetable *t)
{
	assert(t);

	struct sparsetable_iter it;
	it.table = t;
	sparsetable_iter_reset(&it);
	return it;
}

void sparsetable_iter_reset(struct sparsetable_iter *it)
{
	size_t elt_size = sparsetable_elt_size(it->table);

	it->index = -1;
	it->group = it->table->groups;
	it->group_it = sparsegroup_iter_make(it->group, elt_size);
}

bool sparsetable_iter_advance(struct sparsetable_iter *it)
{
	size_t elt_size = sparsetable_elt_size(it->table);
	bool group_adv;
	ssize_t group_idx0, skip;

	if (it->index == sparsetable_size(it->table))
		return false;

	group_idx0 = SPARSEGROUP_IDX(it->group_it);
	group_adv =
	    sparsegroup_iter_advance(it->group, &it->group_it, elt_size);

	while (!group_adv) {
		it->index += sparsegroup_size(it->group) - group_idx0 - 1;

		if (it->index < sparsetable_size(it->table)) {
			it->group++;
			it->group_it =
			    sparsegroup_iter_make(it->group, elt_size);
			group_idx0 = -1;
			group_adv = sparsegroup_iter_advance(it->group,
							     &it->group_it,
							     elt_size);
		} else {
			it->index = sparsetable_size(it->table);
			return false;
		}
	}

	skip = SPARSEGROUP_IDX(it->group_it) - group_idx0;
	it->index += skip;
	assert(skip > 0);
	return true;
}

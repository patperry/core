//  Copyright 2015 Patrick O. Perry.
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.

#ifndef INTSET_H
#define INTSET_H

struct intset {
	int64_t *vals;
	size_t n;
	size_t nmax;
};

// create, destroy
int intset_init(struct intset *s);
int intset_init_copy(struct intset *s, const struct intset *src);
int intset_assign_copy(struct intset *s, const struct intset *src);
int intset_assign_array(struct intset *s, const int64_t *vals, size_t n,
			int sorted);
void intset_destroy(struct intset *s);

// properties
static inline size_t intset_count(const struct intset *s);
static inline size_t intset_capacity(const struct intset *s);
static inline void intset_get_vals(const struct intset *s,
				   const int64_t **vals, size_t *n);

// methods
int intset_add(struct intset *s, int64_t val);
int intset_clear(struct intset *s);
int intset_contains(const struct intset *s, int64_t val);
int intset_remove(struct intset *s, int64_t val);
int intset_ensure_capacity(struct intset *s, size_t n);
int intset_trim_excess(struct intset *s);

// index-based operations
int intset_find(const struct intset *s, int64_t val, size_t *index);
int intset_insert(struct intset *s, size_t index, int64_t val);
int intset_remove_at(struct intset *s, size_t index);

// inline method definitions
size_t intset_count(const struct intset *s)
{
	return s->n;
}

size_t intset_capacity(const struct intset *s)
{
	return s->nmax;
}

void intset_get_vals(const struct intset *s, const int64_t **vals, size_t *n)
{
	*vals = s->vals;
	*n = s->n;
}

#endif // INTSET_H

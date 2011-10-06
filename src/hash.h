#ifndef HASHSET_HASH_H
#define HASHSET_HASH_H

/* Copyright 2005-2009 Daniel James.
 * Copyright 2011 Patrick Perry.
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 * 
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <stddef.h>
#include <stdint.h>


static inline size_t double_hash(double x);
static inline size_t float_hash(float x);
static inline size_t ptr_hash(void *x);

static inline size_t hash_combine(size_t seed, size_t hash);



/* from boost/functional/hash/detail/hash_float_x86.hpp */
size_t float_hash(float val)
{
	union { float f; uint32_t u; } v = { val };
	return v.u;
}

/* from boost/functional/hash/detail/hash_float_x86.hpp */
size_t double_hash(double val)
{
	if (sizeof(double) == sizeof(size_t)) {
		union { double d; size_t i; } v = { val };
		return v.i;
	} else {
		union {double d; uint32_t u[2]; } v = { val };
		size_t seed = v.u[0];
		size_t hash = v.u[1];
		seed ^= hash + (seed << 6) + (seed >> 2);
		return seed;
	}
}

size_t ptr_hash(void *x)
{
	return x >> 2; /* first two bits are typically 0 */
}

/* from boost/functional/hash/hash.hpp */
size_t hash_combine(size_t seed, size_t hash)
{
	seed ^= hash + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	return seed;
}

#endif /* HASHSET_HASH_H */

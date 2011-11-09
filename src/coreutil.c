#include <assert.h>
#include "coreutil.h"

size_t array_grow(size_t count, size_t capacity, size_t delta,
		  size_t capacity_max)
{
	assert(count <= capacity);
	assert(capacity <= capacity_max);
	assert(delta <= capacity_max - count);

	size_t capacity_min = count + delta;
	while (capacity < capacity_min) {
		capacity = ARRAY_GROW1(capacity, capacity_max);
	}

	return capacity;
}

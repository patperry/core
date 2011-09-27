#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include "hashset.h"

#define DEFAULT_ITERS 10000000


struct pair {
	int key;
	int val;
};

static size_t pair_khash(const void *x)
{
	int key = ((struct pair *)x)->key;
	return key;
}

static int pair_kcompar(const void *x1, const void *x2)
{
	int key1 = ((struct pair *)x1)->key;
	int key2 = ((struct pair *)x2)->key;
	return key1 - key2;
}

static void report(char const* title, int iters,
		   const struct rusage *start, const struct rusage *finish)
{
	struct timeval result;
	result.tv_sec  = finish->ru_utime.tv_sec  - start->ru_utime.tv_sec;
	result.tv_usec = finish->ru_utime.tv_usec - start->ru_utime.tv_usec;

	double t = (double)result.tv_sec + (double) result.tv_usec / 1000000.0;

	printf("%-20s %6.1f ns\n", title, (t * 1000000000.0 / iters));
	fflush(stdout);
}

static void time_map_grow(int iters)
{
	struct hashset set;
	struct rusage start, finish;
	struct pair pair;

	hashset_init(&set, sizeof(struct pair), pair_khash, pair_kcompar);
	getrusage(RUSAGE_SELF, &start);
	
	for (pair.key = 0; pair.key < iters; pair.key++) {
		pair.val = pair.key + 1;	
		hashset_set_item(&set, &pair);
	} 

	getrusage(RUSAGE_SELF, &finish);

  	report("map_grow", iters, &start, &finish);
	hashset_deinit(&set);
}

static void time_map_grow_predicted(int iters)
{
	struct hashset set;
	struct rusage start, finish;
	struct pair pair;


	hashset_init(&set, sizeof(struct pair), pair_khash, pair_kcompar);
	hashset_ensure_capacity(&set, iters);
	getrusage(RUSAGE_SELF, &start);
	
	for (pair.key = 0; pair.key < iters; pair.key++) {
		pair.val = pair.key + 1;	
		hashset_set_item(&set, &pair);
	} 

	getrusage(RUSAGE_SELF, &finish);

  	report("map_predict/grow", iters, &start, &finish);
	hashset_deinit(&set);
}

static void time_map_replace(int iters)
{
	struct hashset set;
	struct rusage start, finish;
	struct pair pair;

	hashset_init(&set, sizeof(struct pair), pair_khash, pair_kcompar);
	for (pair.key = 0; pair.key < iters; pair.key++) {
		pair.val = pair.key + 1;	
		hashset_set_item(&set, &pair);
	} 

	getrusage(RUSAGE_SELF, &start);
	
	for (pair.key = 0; pair.key < iters; pair.key++) {
		pair.val = pair.key + 1;	
		hashset_set_item(&set, &pair);
	} 

	getrusage(RUSAGE_SELF, &finish);

  	report("map_replace", iters, &start, &finish);
	hashset_deinit(&set);
}

static void time_map_fetch(int iters, const int *indices, char const* title)
{
	struct hashset set;
	struct rusage start, finish;
	struct pair pair;
	int r, i;

	hashset_init(&set, sizeof(struct pair), pair_khash, pair_kcompar);
	for (pair.key = 0; pair.key < iters; pair.key++) {
		pair.val = pair.key + 1;	
		hashset_set_item(&set, &pair);
	} 

	r = 1;

	getrusage(RUSAGE_SELF, &start);
		
	for (i = 0; i < iters; i++) {
		r ^= (int)(hashset_item(&set, &indices[i]) != NULL);
	}

	getrusage(RUSAGE_SELF, &finish);
	srand(r);   // keep compiler from optimizing away r
	report(title, iters, &start, &finish);
	hashset_deinit(&set);
}


static void time_map_fetch_sequential(int iters)
{
	int *v = malloc(iters * sizeof(v[0]));
	int i;

	for (i = 0; i < iters; i++) {
		v[i] = i;
	}
  	time_map_fetch(iters, v, "map_fetch_sequential");
	free(v);
}

// Apply a pseudorandom permutation to the given vector.
static void shuffle(int *v, int n)
{
	int i1, i2;
	int tmp;

	srand(9);
	for (; n >= 2; n--) {
		i1 = n - 1;
		i2 = (unsigned)(rand() % n);

		tmp = v[i1];
		v[i1] = v[i2];
		v[i2] = tmp;
  	}
}

static void time_map_fetch_random(int iters)
{
	int *v = malloc(iters * sizeof(v[0]));
	int i;

	for (i = 0; i < iters; i++) {
		v[i] = i;
	}
	shuffle(v, iters);
  	time_map_fetch(iters, v, "map_fetch_random");
	free(v);
}

static void time_map_fetch_empty(int iters) {
	struct hashset set;
	struct rusage start, finish;
  	int r;
	int i;

	hashset_init(&set, sizeof(struct pair), pair_khash, pair_kcompar);
	r = 1;
	getrusage(RUSAGE_SELF, &start);

  	for (i = 0; i < iters; i++) {
    		r ^= (int)(hashset_item(&set, &i) != NULL);
  	}

	getrusage(RUSAGE_SELF, &finish);
  	srand(r);   // keep compiler from optimizing away r
	report("map_fetch_empty", iters, &start, &finish);
	hashset_deinit(&set);
}


static void time_map_remove(int iters)
{
	struct hashset set;
	struct rusage start, finish;
	struct pair pair;
	int i;

	hashset_init(&set, sizeof(struct pair), pair_khash, pair_kcompar);
	for (pair.key = 0; pair.key < iters; pair.key++) {
		pair.val = pair.key + 1;	
		hashset_set_item(&set, &pair);
	} 

	getrusage(RUSAGE_SELF, &start);
		
	for (i = 0; i < iters; i++) {
		hashset_remove(&set, &i);
	}

	getrusage(RUSAGE_SELF, &finish);
	report("map_remove", iters, &start, &finish);
	hashset_deinit(&set);
}


static void time_map_toggle(int iters)
{
	struct hashset set;
	struct rusage start, finish;
	struct pair pair, *existing;
	struct hashset_pos pos;

	hashset_init(&set, sizeof(struct pair), pair_khash, pair_kcompar);

	getrusage(RUSAGE_SELF, &start);

#if 0
	for (pair.key = 0; pair.key < iters; pair.key++) {
		pair.val = pair.key + 1;
		hashset_set_item(&set, &pair);
		hashset_remove(&set, &pair);
	}
#else
	for (pair.key = 0; pair.key < iters; pair.key++) {
		if ((existing = hashset_find(&set, &pair, &pos))) {
			existing->val = pair.key + 1;
		} else {
			pair.val = pair.key + 1;
			hashset_insert(&set, &pos, &pair);
		}

		hashset_remove_at(&set, &pos);
	} 
#endif

	getrusage(RUSAGE_SELF, &finish);

  	report("map_toggle", iters, &start, &finish);
}


int main(int argc, char** argv)
{

	int iters = DEFAULT_ITERS;
	if (argc > 1) {            // first arg is # of iterations
		iters = atoi(argv[1]);
	}

	time_map_grow(iters);
	time_map_grow_predicted(iters);
	time_map_replace(iters);
	time_map_fetch_random(iters);
	time_map_fetch_sequential(iters);
	time_map_fetch_empty(iters);
	time_map_remove(iters);
	time_map_toggle(iters);
}


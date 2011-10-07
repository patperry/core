#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include "cmockery.h"

#include "xalloc.h"
#include "pqueue.h"


static int compar(const void *x, const void *y)
{
	return *(int *)x - *(int *)y;
}

static int *base;
static size_t nel;
static size_t capacity;
static int *vals;		// elements sorted in decending order
static size_t nvals;

static void empty_setup_fixture()
{
	print_message("Empty pqueue\n");
	print_message("------------\n");
}

static void empty_setup()
{
	static int *empty_vals = NULL;

	nel = 0;
	capacity = 1024;
	base = xmalloc(capacity * sizeof(base[0]));
	vals = empty_vals;
}

static void singleton_setup_fixture()
{
	print_message("Singleton pqueue\n");
	print_message("----------------\n");
}

static void singleton_setup()
{
	static int singleton_vals[] = { 1234 };

	empty_setup();
	vals = singleton_vals;
	nvals = 1;

	pqueue_push(&vals[0], base, &nel, sizeof(base[0]), compar);
}

static void sorted5_setup_fixture()
{
	print_message("Pqueue with 5 sorted elements\n");
	print_message("-----------------------------\n");
}

static void sorted5_setup()
{
	static int sorted5_vals[] = { 5, 4, 3, 2, 1 };
	
	empty_setup();
	vals = sorted5_vals;
	nvals = 5;

	int i, n = nvals;
	
	for (i = 0; i < n; i++)
		pqueue_push(&vals[i], base, &nel, sizeof(base[0]), compar);
}

static void unsorted7_setup_fixture()
{
	print_message("Pqueue with 7 unsorted elements\n");
	print_message("-------------------------------\n");
}

static void unsorted7_setup()
{
	static int sorted7_vals[] = { 7, 6, 5, 4, 3, 2, 1 };
	int unsorted7_vals[] = { 2, 1, 3, 4, 7, 6, 5 };

	empty_setup();
	vals = sorted7_vals;
	nvals = 7;
	
	int i, n = nvals;

	for (i = 0; i < n; i++)
		pqueue_push(&unsorted7_vals[i], base, &nel, sizeof(base[0]),
				compar);
}

static void teardown()
{
	free(base);
}

static void teardown_fixture()
{
	print_message("\n\n");
}

static void test_count()
{
	assert_int_equal(nel, nvals);
}

static void test_push_min_minus_one()
{
	int min = vals[nvals - 1];
	int min_minus_one = min - 1;
	pqueue_push(&min_minus_one, base, &nel, sizeof(base[0]), compar);
	assert_int_equal(nel, nvals + 1);
	assert_int_equal(base[0], vals[0]);
}

static void test_push_min()
{
	int min = vals[nvals - 1];
	pqueue_push(&min, base, &nel, sizeof(base[0]), compar);
	assert_int_equal(nel, nvals + 1);
	assert_int_equal(base[0], vals[0]);
}

static void test_push_max_minus_one()
{
	int max = vals[0];
	int max_minus_one = max - 1;
	pqueue_push(&max_minus_one, base, &nel, sizeof(base[0]), compar);
	assert_int_equal(nel, nvals + 1);
	assert_int_equal(base[0], max);
}

static void test_push_max()
{
	int max = vals[0];
	pqueue_push(&max, base, &nel, sizeof(base[0]), compar);
	assert_int_equal(nel, nvals + 1);
	assert_int_equal(base[0], max);
}

static void test_push_max_plus_one()
{
	int max = vals[0];
	int max_plus_one = max + 1;
	pqueue_push(&max_plus_one, base, &nel, sizeof(base[0]), compar);
	assert_int_equal(nel, nvals + 1);
	assert_int_equal(base[0], max + 1);
}

static void test_push_existing()
{
	size_t i, j;
	int top;
	int val;
	int *copy = xcalloc(capacity, sizeof(copy[0]));
	size_t copy_nel;

	for (i = 0; i < nel; i++) {
		memcpy(copy, base, nel * sizeof(copy[0]));
		copy_nel = nel;
		val = vals[i];

		pqueue_push(&val, copy, &copy_nel, sizeof(copy[0]), compar);

		assert_int_equal(copy_nel, nel + 1);

		for (j = 0; j < nel + 1; j++) {
			top = copy[0];
			pqueue_pop(copy, &copy_nel, sizeof(copy[0]), compar);

			if (j <= i) {
				assert_int_equal(top, vals[j]);
			} else {
				assert_int_equal(top, vals[j - 1]);
			}
		}
	}

	free(copy);
}

int main()
{
	UnitTest tests[] = {
		unit_test_setup(empty_suite, empty_setup_fixture),
		unit_test_setup_teardown(test_count, empty_setup, teardown),
		unit_test_teardown(empty_suite, teardown_fixture),

		unit_test_setup(singleton_suite, singleton_setup_fixture),
		unit_test_setup_teardown(test_count, singleton_setup, teardown),
		unit_test_setup_teardown(test_push_min, singleton_setup,
					 teardown),
		unit_test_setup_teardown(test_push_min_minus_one,
					 singleton_setup, teardown),
		unit_test_setup_teardown(test_push_max, singleton_setup,
					 teardown),
		unit_test_setup_teardown(test_push_max_minus_one,
					 singleton_setup, teardown),
		unit_test_setup_teardown(test_push_max_plus_one,
					 singleton_setup, teardown),
		unit_test_setup_teardown(test_push_existing, singleton_setup,
					 teardown),
		unit_test_teardown(singleton_suite, teardown_fixture),

		unit_test_setup(sorted5_suite, sorted5_setup_fixture),
		unit_test_setup_teardown(test_count, sorted5_setup, teardown),
		unit_test_setup_teardown(test_push_min, sorted5_setup,
					 teardown),
		unit_test_setup_teardown(test_push_min_minus_one, sorted5_setup,
					 teardown),
		unit_test_setup_teardown(test_push_max, sorted5_setup,
					 teardown),
		unit_test_setup_teardown(test_push_max_minus_one, sorted5_setup,
					 teardown),
		unit_test_setup_teardown(test_push_max_plus_one, sorted5_setup,
					 teardown),
		unit_test_setup_teardown(test_push_existing, sorted5_setup,
					 teardown),
		unit_test_teardown(sorted5_suite, teardown_fixture),

		unit_test_setup(unsorted7_suite, unsorted7_setup_fixture),
		unit_test_setup_teardown(test_count, unsorted7_setup, teardown),
		unit_test_setup_teardown(test_push_min, unsorted7_setup,
					 teardown),
		unit_test_setup_teardown(test_push_min_minus_one,
					 unsorted7_setup, teardown),
		unit_test_setup_teardown(test_push_max, unsorted7_setup,
					 teardown),
		unit_test_setup_teardown(test_push_max_minus_one,
					 unsorted7_setup, teardown),
		unit_test_setup_teardown(test_push_max_plus_one,
					 unsorted7_setup, teardown),
		unit_test_setup_teardown(test_push_existing, unsorted7_setup,
					 teardown),
		unit_test_teardown(unsorted7_suite, teardown_fixture),

	};
	return run_tests(tests);
}

#include <assert.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include "cmockery.h"

#include "pqueue.h"

int compar(const void *x, const void *y, void *context)
{
	(void)context;
	return *(int *)x - *(int *)y;
}

static struct pqueue pqueue;
static size_t count;
static int *elts;		// elements sorted in decending order

static void empty_setup_fixture()
{
	print_message("Empty pqueue\n");
	print_message("------------\n");
}

static void empty_setup()
{
	static int *empty_elts = NULL;

	pqueue_init(&pqueue, sizeof(int), compar, NULL);
	count = 0;
	elts = empty_elts;
}

static void singleton_setup_fixture()
{
	print_message("Singleton pqueue\n");
	print_message("----------------\n");
}

static void singleton_setup()
{
	static int singleton_elts[] = { 1234 };
	void *ptr;

	pqueue_init(&pqueue, sizeof(int), compar, NULL);
	elts = singleton_elts;
	count = 1;
	pqueue_push(&pqueue, elts);
}

static void sorted5_setup_fixture()
{
	print_message("Pqueue with 5 sorted elements\n");
	print_message("-----------------------------\n");
}

static void sorted5_setup()
{
	static int sorted5_elts[] = { 5, 4, 3, 2, 1 };
	size_t i;
	
	pqueue_init(&pqueue, sizeof(int), compar, NULL);
	elts = sorted5_elts;
	count = 5;
	
	for (i = 0; i < count; i++)
		pqueue_push(&pqueue, &elts[i]);

}

static void unsorted7_setup_fixture()
{
	print_message("Pqueue with 7 unsorted elements\n");
	print_message("-------------------------------\n");
}

static void unsorted7_setup()
{
	static int sorted7_elts[] = { 7, 6, 5, 4, 3, 2, 1 };
	int unsorted7_elts[] = { 2, 1, 3, 4, 7, 6, 5 };
	size_t i;

	pqueue_init(&pqueue, sizeof(int), compar, NULL);
	elts = sorted7_elts;
	count = 7;
	
	for (i = 0; i < count; i++)
		pqueue_push(&pqueue, &unsorted7_elts[i]);
}

static void teardown()
{
	pqueue_destroy(&pqueue);
}

static void teardown_fixture()
{
	print_message("\n\n");
}

static void test_count()
{
	assert_int_equal(pqueue_count(&pqueue), count);
}

static void test_push_min_minus_one()
{
	int min = elts[count - 1];
	int min_minus_one = min - 1;
	pqueue_push(&pqueue, &min_minus_one);
	assert_int_equal(pqueue_count(&pqueue), count + 1);
	assert_int_equal(*(int *)pqueue_top(&pqueue), elts[0]);
}

static void test_push_min()
{
	int min = elts[count - 1];
	int elt = min;
	pqueue_push(&pqueue, &elt);
	assert_int_equal(pqueue_count(&pqueue), count + 1);
	assert_int_equal(*(int *)pqueue_top(&pqueue), elts[0]);
}

static void test_push_max_minus_one()
{
	int max = elts[0];
	int max_minus_one = max - 1;
	pqueue_push(&pqueue, &max_minus_one);
	assert_int_equal(pqueue_count(&pqueue), count + 1);
	assert_int_equal(*(int *)pqueue_top(&pqueue), max);
}

static void test_push_max()
{
	int max = elts[0];
	int elt = max;
	pqueue_push(&pqueue, &elt);
	assert_int_equal(pqueue_count(&pqueue), count + 1);
	assert_int_equal(*(int *)pqueue_top(&pqueue), max);
}

static void test_push_max_plus_one()
{
	int max = (count ? elts[0] : 0);
	int max_plus_one = max + 1;
	pqueue_push(&pqueue, &max_plus_one);
	assert_int_equal(pqueue_count(&pqueue), count + 1);
	assert_int_equal(*(int *)pqueue_top(&pqueue), max + 1);
}

static void test_push_existing()
{
	size_t i, j;
	int top;
	struct pqueue pq;
	int elt;

	for (i = 0; i < count; i++) {
		pqueue_init_copy(&pq, &pqueue);
		elt = elts[i];

		pqueue_push(&pq, &elt);

		assert_int_equal(pqueue_count(&pq), count + 1);

		for (j = 0; j < count + 1; j++) {
			top = *(int *)pqueue_top(&pq);
			pqueue_pop(&pq);
			if (j <= i) {
				assert_int_equal(top, elts[j]);
			} else {
				assert_int_equal(top, elts[j - 1]);
			}
		}

		pqueue_destroy(&pq);
	}
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

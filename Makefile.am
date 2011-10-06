# Process this file with automake to produce Makefile.in

AM_CPPFLAGS = -Ilib -Isrc

noinst_LIBRARIES = \
		libcore.a 

libcore_a_SOURCES = \
		src/hash.h \
		src/hashset.c \
		src/hashset.h \
		src/timsort-impl.h \
		src/timsort.c \
		src/timsort.h \
		src/xalloc-die.c \
		src/xalloc.c \
		src/xalloc.h

check_LIBRARIES = \
		tests/libcmockery.a

tests_libcmockery_a_SOURCES = \
		tests/cmockery.c \
		tests/cmockery.h

check_PROGRAMS = \
		tests/hashset-test \
		tests/hashset-benchmark

tests_hashset_test_LDADD = \
		libcore.a \
		tests/libcmockery.a \
		$(LIBS)

tests_hashset_benchmark_LDADD = \
		libcore.a \
		$(LIBS)

TESTS = $(check_PROGRAMS)

# Process this file with automake to produce Makefile.in

AM_CPPFLAGS = -Ilib -Isrc

noinst_LIBRARIES = \
		libhashset.a \
		libshashset.a

libhashset_a_SOURCES = \
		src/hash.h \
		src/hashset.c \
		src/hashset.h \
		lib/xalloc-die.c \
		lib/xalloc.c \
		lib/xalloc.h

libshashset_a_SOURCES = \
		src/shashset.c \
		src/shashset.h \
		src/sparsegroup.c \
		src/sparsegroup.h \
		src/sparsetable.c \
		src/sparsetable.h \
		lib/xalloc-die.c \
		lib/xalloc.c \
		lib/xalloc.h

check_LIBRARIES = \
		tests/libcmockery.a

tests_libcmockery_a_SOURCES = \
		tests/cmockery.c \
		tests/cmockery.h

check_PROGRAMS = \
		tests/hashset-test \
		tests/benchmark

tests_hashset_test_LDADD = \
		libhashset.a \
		tests/libcmockery.a \
		$(LIBS)

tests_benchmark_LDADD = \
		libhashset.a \
		$(LIBS)

TESTS = $(check_PROGRAMS)

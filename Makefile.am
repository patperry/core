# Process this file with automake to produce Makefile.in

noinst_LIBRARIES = \
		libhashset.a

libhashset_a_SOURCES = \
		hashset.c \
		hashset.h \
		shashset.c \
		shashset.h \
		sparsegroup.c \
		sparsegroup.h \
		sparsetable.c \
		sparsetable.h \
		xalloc-die.c \
		xalloc.c \
		xalloc.h

check_LIBRARIES = \
		libcmockery.a

libcmockery_a_SOURCES = \
		cmockery.c \
		cmockery.h

check_PROGRAMS = \
		hashset-test

hashset_test_LDADD = \
		libhashset.a \
		libcmockery.a \
		$(LIBS)

TESTS = $(check_PROGRAMS)

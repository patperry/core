dnl Process this file with autoconf to produce a configure script.

AC_INIT([core], [1.0], [patperry@gmail.com])
AC_CONFIG_AUX_DIR([build-aux])

AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

# Minimum Autoconf version required.
AC_PREREQ(2.60)

# Where to generate output; srcdir location.
AC_CONFIG_HEADERS([config.h:config.in])dnl Keep filename to 8.3 for MS-DOS.
AC_CONFIG_SRCDIR([src/hashset.c])

dnl Checks for programs.
AC_PROG_CC
AC_PROG_RANLIB

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

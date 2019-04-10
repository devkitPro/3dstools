#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT([3dstools],[1.2.0],[https://github.com/devkitPro/3dstools/issues])
AC_CONFIG_SRCDIR([src/3dsxtool.cpp])

AM_INIT_AUTOMAKE([subdir-objects])

AC_CANONICAL_BUILD
AC_CANONICAL_HOST

AC_PROG_CC
AC_PROG_CXX

AC_SYS_LARGEFILE
AC_FUNC_FSEEKO

AS_CASE([$host_os],
 [mingw32*],
  [AC_DEFINE([__USE_MINGW_ANSI_STDIO], [1], [Set to 1 to use C99 printf/snprintf in MinGW.])
 ],
 [])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

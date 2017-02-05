#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([testsuit], [0.0.1], [catchgreen@163.com])
AC_CONFIG_SRCDIR([common_sz.h])
AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE(foreign)
# Checks for programs.
AC_PROG_CC
AC_PROG_RANLIB

# Checks for libraries.
AC_SEARCH_LIBS(pthread_create, pthread)
AC_SEARCH_LIBS(sqlite3_close, sqlite3)
# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h fcntl.h netinet/in.h stdlib.h string.h sys/socket.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_MKTIME
AC_CHECK_FUNCS([memset select socket strerror])
AC_CONFIG_FILES(Makefile)
AC_OUTPUT

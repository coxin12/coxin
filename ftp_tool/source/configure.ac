#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

#AC_PREREQ([2.63])
AC_INIT([FileTransfer], [V14.00.001], [liangjianqiang@revenco.com])
AM_INIT_AUTOMAKE([foreign])
AC_CONFIG_SRCDIR([src/main.cpp])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

# Checks for libraries.
AC_CHECK_LIB([desini], [main])
AC_CHECK_LIB([logger], [main])
AC_CHECK_LIB([pthread], [main])
AC_CHECK_LIB([ssh2], [main])

# Checks for header files.
AC_CHECK_HEADERS([memory.h stdlib.h string.h strings.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_TYPE_INT64_T
AC_TYPE_MODE_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_FORK
AC_FUNC_LSTAT_FOLLOWS_SLASHED_SYMLINK
AC_FUNC_MALLOC
AC_CHECK_FUNCS([localtime_r memset mkdir regcomp rmdir select socket strcasecmp strchr strdup strerror strncasecmp strstr])

AC_CONFIG_FILES([Makefile
                 src/Makefile
                 etc/Makefile
                 docs/Makefile])
AC_OUTPUT

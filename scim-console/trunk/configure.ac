#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([src/scim_console.cpp])
AM_INIT_AUTOMAKE(scim-console, 0.1.0, ryo-dairiki@users.sourceforge.net)
AC_CONFIG_HEADER([config.h])

###########################################################
##             Fundamental compile options.              ##
###########################################################

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL

# Checks for libraries.

# Checks for header files.
AC_FUNC_ALLOCA
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([fcntl.h locale.h stdlib.h string.h sys/ioctl.h sys/socket.h sys/time.h unistd.h utmp.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_TYPE_MODE_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T

# Checks for library functions.
AC_PROG_GCC_TRADITIONAL
AC_FUNC_SELECT_ARGTYPES
AC_CHECK_FUNCS([memset select setlocale socket strerror wcwidth])

###########################################################
##            Checking for building conditions.          ##
###########################################################
PKG_CHECK_MODULES(scim, scim >= 1.4.0)
AC_CHECK_HEADER(curses.h, , [AC_MSG_ERROR(['curses' library is not found])])
AC_CHECK_HEADER(rote/rote.h, , [AC_MSG_ERROR(['rote' library is not found])])

###########################################################
##             Definiation of compile args.              ##
###########################################################

SCIM_VERSION=`$PKG_CONFIG --modversion scim`
SCIM_BINARY_VERSION=`$PKG_CONFIG --variable=scim_binary_version scim`
SCIM_LIBDIR=`$PKG_CONFIG --variable=libdir scim`
SCIM_CFLAGS=`$PKG_CONFIG --cflags scim`
SCIM_LIBS=`$PKG_CONFIG --libs scim`

if test "x$SCIM_LIBEXECDIR" = "x"; then
  SCIM_LIBEXECDIR="${SCIM_LIBDIR}/scim-1.0/"
fi

AC_SUBST(SCIM_BINARY_VERSION)
AC_SUBST(SCIM_VERSION)
AC_SUBST(SCIM_LIBS)
AC_SUBST(SCIM_CFLAGS)
AC_SUBST(SCIM_LIBEXECDIR)

CURSES_CFLAGS=""
CURSES_LIBS="-lncursesw"

AC_SUBST(CURSES_CFLAGS)
AC_SUBST(CURSES_LIBS)

UTIL_CFLAGS=""
UTILS_LIBS="-lutil"

AC_SUBST(UTIL_CFLAGS)
AC_SUBST(UTIL_LIBS)

ROTE_CFLAGS=""
ROTE_LIBS="-lrote"

AC_SUBST(ROTE_CFLAGS)
AC_SUBST(ROTE_LIBS)

AC_OUTPUT(Makefile src/Makefile)


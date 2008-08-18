# Process this file with autoconf to produce a configure script.
AC_INIT(scim-cns11643-filter, 0.0.2, suzhe@tsinghua.org.cn)
AC_CONFIG_SRCDIR([src/scim_cns11643_filter.h])

SCIM_VERSION=1.4.0

AC_SUBST(SCIM_VERSION)

GETTEXT_PACKAGE=scim-cns11643-filter
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [The gettext domain])

# Init automake stuff
AM_INIT_AUTOMAKE
AM_CONFIG_HEADER([config.h])

# Init gettext
ALL_LINGUAS=""
AM_GNU_GETTEXT


AC_LIBTOOL_WIN32_DLL
AC_LIBTOOL_DLOPEN
AC_PROG_LIBTOOL
AC_SUBST(LIBTOOL_DEPS)

AC_PROG_CXX
AC_PROG_CC
AC_LANG(C++)

# Check SCIM 
PKG_CHECK_MODULES(SCIM,[scim >= $SCIM_VERSION])

# Checks for header files.
AC_HEADER_STDC

SCIM_ICONDIR=`$PKG_CONFIG --variable=icondir scim`
SCIM_MODULEDIR=`$PKG_CONFIG --variable=moduledir scim`
SCIM_DATADIR=`$PKG_CONFIG --variable=scimdatadir scim`
SCIM_LIBEXECDIR=`$PKG_CONFIG --variable=moduledir scim`

if test "x$SCIM_ICONDIR" = "x"; then
  SCIM_ICONDIR=${datadir}/scim/icons
fi

if test "x$SCIM_MODULEDIR" = "x"; then
  SCIM_MODULEDIR=${libdir}/scim-1.0
fi

if test "x$SCIM_DATADIR" = "x"; then
  SCIM_DATADIR=${datadir}/scim
fi

if test "x$SCIM_LIBEXECDIR" = "x"; then
  SCIM_LIBEXECDIR=${libdir}/scim-1.0
fi

AC_SUBST(SCIM_ICONDIR)
AC_SUBST(SCIM_MODULEDIR)
AC_SUBST(SCIM_DATADIR)
AC_SUBST(SCIM_LIBEXECDIR)

AC_CONFIG_FILES([Makefile intl/Makefile po/Makefile.in m4/Makefile
		 src/Makefile
		 scim-cns11643-filter.spec])
AC_OUTPUT

# Process this file with autoconf to produce a configure script.
AC_INIT(scim-input-pad, 0.1.1, suzhe@tsinghua.org.cn)
AC_CONFIG_SRCDIR([src/scim_input_pad.cpp])

SCIM_VERSION=1.2.0
GTK_VERSION=2.4.0

AC_SUBST(SCIM_VERSION)
AC_SUBST(GTK_VERSION)

# libtool versioning for SCIM Input Pad

# increment if the interface has additions, changes, removals.
SCIM_INPUT_PAD_CURRENT=0

# increment any time the source changes; set to 0 if you increment CURRENT
SCIM_INPUT_PAD_REVISION=0

# increment if any interfaces have been added; set to 0
# if any interfaces have been removed. removal has 
# precedence over adding, so set to 0 if both happened.
SCIM_INPUT_PAD_AGE=0

AC_SUBST(SCIM_INPUT_PAD_CURRENT)
AC_SUBST(SCIM_INPUT_PAD_REVISION)
AC_SUBST(SCIM_INPUT_PAD_AGE)

GETTEXT_PACKAGE=scim-input-pad
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [The gettext domain])

# Init automake stuff
AM_INIT_AUTOMAKE
AM_CONFIG_HEADER([config.h])

# Init gettext
ALL_LINGUAS="zh_CN zh_TW pa ja de fr it"
AM_GNU_GETTEXT

# Check intltool
IT_PROG_INTLTOOL([0.33], [no-xml])

AC_LIBTOOL_WIN32_DLL
AC_LIBTOOL_DLOPEN
AC_PROG_LIBTOOL
AC_LIB_LTDL
AC_SUBST(LIBTOOL_DEPS)

AC_PROG_CXX
AC_PROG_CC
AC_LANG(C++)

AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T

# Check SCIM 
PKG_CHECK_MODULES(SCIM,[scim >= $SCIM_VERSION])

# Check gtk2
PKG_CHECK_MODULES(GTK2, [gtk+-2.0 >= 2.4.0])

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([libintl.h dirent.h])

# libtool option to control which symbols are exported
# right now, symbols starting with _ are not exported
LIBTOOL_EXPORT_OPTIONS='-export-symbols-regex "^[[^_]].*"'
AC_SUBST(LIBTOOL_EXPORT_OPTIONS)

# Extra args.
AC_ARG_ENABLE(debug,
	      [  --enable-debug            Turn on debugging],
       	      enable_debug=yes,
	      enable_debug=no)

if test "$enable_debug" = "yes"; then
  AC_DEFINE(ENABLE_DEBUG,1,[Define this to enable the debug facility in libscim])
  CFLAGS="$CFLAGS -g"
  CXXFLAGS="$CXXFLAGS -g"
fi

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

SCIM_INPUT_PAD_DATADIR=$SCIM_DATADIR/input-pad
SCIM_INPUT_PAD_USER_DATADIR=/.scim/input-pad

AC_SUBST(SCIM_INPUT_PAD_DATADIR)
AC_SUBST(SCIM_INPUT_PAD_USER_DATADIR)

AC_CONFIG_FILES([Makefile intl/Makefile po/Makefile.in m4/Makefile 
		 data/scim-input-pad
		 data/Makefile
		 src/Makefile
		 scim-input-pad.spec])
AC_OUTPUT

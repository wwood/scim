# Process this file with autoconf to produce a configure script.
AC_INIT(scim-pinyin, 0.5.91, suzhe@tsinghua.org.cn) #do not forget to edit skim/configure.in.in to change the version
AC_CONFIG_SRCDIR([src/scim_pinyin_imengine.h])

SCIM_VERSION=1.2.0
AC_SUBST(SCIM_VERSION)

GETTEXT_PACKAGE=scim-pinyin
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [The gettext domain])

# Init automake stuff
AM_INIT_AUTOMAKE
AM_CONFIG_HEADER([config.h])

# Init gettext
ALL_LINGUAS="zh_CN zh_TW ko ja de fr it"
AM_GNU_GETTEXT

# Init libtool
AC_LIBTOOL_WIN32_DLL
AC_LIBTOOL_DLOPEN
AC_PROG_LIBTOOL
AC_LIB_LTDL
AC_SUBST(LIBTOOL_DEPS)

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_LANG(C++)

AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T

# Check SCIM
PKG_CHECK_MODULES(SCIM,[scim >= $SCIM_VERSION])

# Check if we should build setup module
PKG_CHECK_MODULES(SCIM_GTKUTILS,[scim-gtkutils >= $SCIM_VERSION],
		  		[SCIM_HAS_GTKUTILS=yes],
				[SCIM_HAS_GTKUTILS=no])

if test "$SCIM_HAS_GTKUTILS" = "yes"; then
  SCIM_BUILD_SETUP=1
else
  SCIM_BUILD_SETUP=0
fi

AM_CONDITIONAL(SCIM_BUILD_SETUP, [test "$SCIM_HAS_GTKUTILS" = "yes"])
AC_SUBST(SCIM_BUILD_SETUP)


# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([langinfo.h libintl.h string.h hash_map ext/hash_map])
AC_HEADER_TIME

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([gettimeofday memmove memset nl_langinfo setlocale strtol])
AC_CHECK_FUNCS([time, localtime, localtime_r])

# libtool option to control which symbols are exported
# right now, symbols starting with _ are not exported
LIBTOOL_EXPORT_OPTIONS='-export-symbols-regex "^[[^_]].*"'
AC_SUBST(LIBTOOL_EXPORT_OPTIONS)

# Extra args.
AC_ARG_ENABLE(debug,
	      [  --enable-debug     Turn on debugging],
       	      enable_debug=yes,
	      enable_debug=no)

AC_ARG_ENABLE(skim-support,
              [  --disable-skim-support  Enable skim Setup UI],
              skim_support=no,
              skim_support=yes)

AM_CONDITIONAL(SCIM_BUILD_SKIM_SETUP, [test "$skim_support" = "yes"])

if test "$enable_debug" = "yes"; then
  AC_DEFINE(ENABLE_DEBUG,1,[Define this to enable the debug facility in libscim])
  CFLAGS="$CFLAGS -g"
  CXXFLAGS="$CXXFLAGS -g"
fi


AC_ARG_ENABLE(tools,
	      [  --enable-tools     Make tools],
	      enable_tools=yes,
	      enable_tools=no)

AM_CONDITIONAL(ENABLE_TOOLS, test "x$tools" = "xyes")


SCIM_ICONDIR=`$PKG_CONFIG --variable=icondir scim`
SCIM_MODULEDIR=`$PKG_CONFIG --variable=moduledir scim`
SCIM_DATADIR=`$PKG_CONFIG --variable=scimdatadir scim`

if test "x$SCIM_ICONDIR" = "x"; then
  SCIM_ICONDIR=${datadir}/scim/icons
fi

if test "x$SCIM_MODULEDIR" = "x"; then
  SCIM_MODULEDIR=${libdir}/scim-1.0
fi

if test "x$SCIM_DATADIR" = "x"; then
  SCIM_DATADIR=${datadir}/scim
fi

AC_SUBST(SCIM_ICONDIR)
AC_SUBST(SCIM_MODULEDIR)
AC_SUBST(SCIM_DATADIR)

SCIM_PINYIN_DATADIR=$SCIM_DATADIR/pinyin

AC_SUBST(SCIM_PINYIN_DATADIR)

AC_CONFIG_FILES([Makefile
                 m4/Makefile
                 src/Makefile
		 data/Makefile
                 intl/Makefile
                 po/Makefile.in
                 scim-pinyin.spec])

if test "$skim_support" = "yes"; then
REQUIRED_SKIM_VERSION=1.2.1
dnl skim probably has been installed under prefix=$KDEDIR, so add it to search path of pkgconfig
kde_libsuffix=`kde-config --libsuffix`
test -z "$KDEDIR" && KDEDIR=`kde-config --prefix`
AC_SUBST(KDEDIR)
export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:$KDEDIR/lib${kde_libsuffix}/pkgconfig
PKG_CHECK_MODULES(SKIM, [skim >= $REQUIRED_SKIM_VERSION],
                        [HAS_SKIM=yes],
                        [HAS_SKIM=no])
else
HAS_SKIM=no
fi

if test "x$HAS_SKIM" = "xyes"; then

PKG_CHECK_MODULES(SCIM_KDEUTILS,[scim_kdeutils >= $REQUIRED_SKIM_VERSION],
                                [HAS_KDEUTILS=yes],
                                [HAS_KDEUTILS=no])

  if test "x$HAS_KDEUTILS" = "xyes"; then
      SCIM_PINYIN_ENABLE_SKIM_SUPPORT=1
      AC_CONFIG_SUBDIRS(skim)
  else
      SCIM_PINYIN_ENABLE_SKIM_SUPPORT=0
  fi
fi

AM_CONDITIONAL(SCIM_PINYIN_ENABLE_SKIM_SUPPORT, [test "$HAS_KDEUTILS" = "yes"])
AC_SUBST(SCIM_PINYIN_ENABLE_SKIM_SUPPORT)
AC_OUTPUT

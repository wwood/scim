#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(ccinput, 0.3.0, chinese@ccoss.com.cn)
AC_CONFIG_SRCDIR([src/pinyin_parse.c])

###########################################################
##                  Start Configuration.                 ##
###########################################################

# Init automake stuff
AM_INIT_AUTOMAKE
AM_CONFIG_HEADER([config.h])

# Init gettext
ALL_LINGUAS="zh_CN zh_TW"
#AM_GNU_GETTEXT

# Init libtool
AC_LIBTOOL_DLOPEN
AC_PROG_LIBTOOL
AC_LIB_LTDL
AC_SUBST(LIBTOOL_DEPS)

# libtool option to control which symbols are exported
# right now, symbols starting with _ are not exported
LIBTOOL_EXPORT_OPTIONS='-export-symbols-regex "^[[^_]].*"'


# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB

# Checks if doxygen is avaiable

AC_PATH_PROG(DOXYGEN, doxygen, no)
AM_CONDITIONAL(HAVE_DOXYGEN, test x$DOXYGEN != xno)
AC_SUBST(DOXYGEN)

# Checks if graphviz is available
AC_CHECK_PROG(HAVE_GRAPHVIZ, dot, YES, NO)
AC_PATH_PROG(DOT, dot)
AC_SUBST(DOT)

AC_PATH_PROG(PERL, perl)
AC_SUBST(PERL)

# Checks if gconftool-2 is available
AC_PATH_PROG(GCONFTOOL, gconftool-2)
AC_SUBST(GCONFTOOL)

# Checks if xsltproc is available
AC_PATH_PROG(XSLTPROC, xsltproc, no)
AM_CONDITIONAL(HAVE_XSLTPROC, test x$XSLTPROC != xno)
AC_SUBST(XSLTPROC)

# Checks if docbook-style-xsl is available
AC_CHECK_FILE(
[/usr/share/sgml/docbook/xsl-stylesheets/html/tldp-html.xsl],
[DOCBOOK_XSL=/usr/share/sgml/docbook/xsl-stylesheets/html/tldp-html.xsl],
[AC_CHECK_FILE(
 [/usr/share/sgml/docbook/xsl-stylesheets/html/docbook.xsl],
 [DOCBOOK_XSL=/usr/share/sgml/docbook/xsl-stylesheets/html/docbook.xsl],
 [DOCBOOK_XSL=no])])
AC_SUBST(DOCBOOK_XSL)

AM_CONDITIONAL(HAVE_DOCBOOK, test x$DOCBOOK_XSL != xno)
AM_CONDITIONAL(BUILD_MANUAL, test x$XSLTPROC != xno -a x$DOCBOOK_XSL != xno)

# Check intltool
#AC_PROG_INTLTOOL([0.22])

# Checks for libraries.
AC_HEADER_STDC
AC_HEADER_TIME
AC_CHECK_HEADERS([langinfo.h libintl.h string.h dirent.h fcntl.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T

AC_CHECK_SIZEOF(char)
AC_CHECK_SIZEOF(short)
AC_CHECK_SIZEOF(wchar_t)

if test x$ac_cv_sizeof_short = x2 && test x$ac_cv_sizeof_char = x1 ; then
  :
else
  AC_MSG_ERROR([
*** SCIM requires
***      sizeof (short)     == 2
***      sizeof (char)      == 1
*** You might want to consider using the GNU C compiler.
])
fi

if test x$ac_cv_sizeof_wchar_t = x0 ; then
  AC_MSG_ERROR([
*** SCIM requires a compiler that supports wchar_t,
*** You might want to consider using the GNU C compiler.
])
fi


# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([gettimeofday memmove memset nl_langinfo setlocale daemon])
AC_CHECK_FUNCS([opendir closedir readdir])
AC_CHECK_FUNCS([gethostbyname gethostbyname_r socket bind accept connect listen],
	       [socket_ok=yes],
	       [socket_ok=no])
AC_CHECK_FUNCS([bzero mkdir strtol])
AC_FUNC_CLOSEDIR_VOID
AC_FUNC_STAT
AC_HEADER_DIRENT
AC_HEADER_STDBOOL

# Checks for libraries.
SCIM_VERSION=1.2.0

PKG_CHECK_MODULES(SCIM,[scim >= $SCIM_VERSION])

# Check if we should build setup module
PKG_CHECK_MODULES(SCIM_GTKUTILS,[scim-gtkutils >= $SCIM_VERSION],
                                [SCIM_HAS_GTKUTILS=yes],
                                [SCIM_HAS_GTKUTILS=no])

install_scim_version=`$PKG_CONFIG --modversion scim`

if [ "x`echo ${install_scim_version} | sed -E 's/^(...).+$/\1/'`" = "x1.2" ]; then
  AC_DEFINE(SCIM_1_2_API,1,[Whether the current scim is 1.2.x])
else
  AC_DEFINE(SCIM_1_2_API,0)
fi

AC_ARG_ENABLE(skim-support,
              [  --disable-skim-support  Enable skim Setup UI],
              skim_support=no,
              skim_support=yes)

AM_CONDITIONAL(SCIM_BUILD_SKIM_SETUP, [test "$skim_support" = "yes"])

AC_SUBST(SCIM_VERSION)

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

###########################################################

AC_CONFIG_FILES([Makefile
		 ccinput.spec
		 configs/ccinput.cfg
                 po/Makefile
                 icons/Makefile
                 data/Makefile
                 configs/Makefile
		 src/Makefile
		 scim/Makefile
                 scim/IMEngine/Makefile
                 scim/SetupUI/Makefile])

if test "$skim_support" = "yes"; then
REQUIRED_SKIM_VERSION=1.2.1
dnl skim probably has been installed under prefix=$KDEDIR, so add it to search path of pkgconfig
test -z "$KDEDIR" && KDEDIR=`kde-config --prefix`
export PKG_CONFIG_PATH=$KDEDIR/lib/pkgconfig
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
      SCIM_TABLES_ENABLE_SKIM_SUPPORT=1
      AC_CONFIG_SUBDIRS(skim)
  else
      SCIM_TABLES_ENABLE_SKIM_SUPPORT=0
  fi
fi

AM_CONDITIONAL(SCIM_TABLES_ENABLE_SKIM_SUPPORT, [test "$HAS_KDEUTILS" = "yes"])
AC_SUBST(SCIM_TABLES_ENABLE_SKIM_SUPPORT)

AC_OUTPUT

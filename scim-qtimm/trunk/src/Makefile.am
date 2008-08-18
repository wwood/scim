# set the include path for X, qt and KDE
INCLUDES = $(all_includes) @SCIM_CFLAGS@ @SCIM_X11_CFLAGS@

# these are the headers for your project
noinst_HEADERS = keyserver_x11.h qsciminputcontext.h

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

#messages: rc.cpp
#	$(XGETTEXT) *.cpp -o $(podir)/qt_immodule.pot

#KDE_ICON = qt_immodule

#########################################################################
# APPLICATION SECTION
#########################################################################
# this is the program that gets installed.  it's name is used for all
# of the other Makefile.am variables

pkglib_LTLIBRARIES =  libqscim.la
pkglibdir=${INPUTMETHODSDIR}

# the application source, library search path, and link libraries
libqscim_la_SOURCES = plugin.cpp qsciminputcontext.cpp
libqscim_la_LDFLAGS = -avoid-version -rpath $(pkglibdir) -module $(all_libraries)
libqscim_la_LIBADD = @SCIM_LIBS@ @SCIM_X11_LIBS@

#KDE_OPTIONS = qtonly

MAINTAINERCLEANFILES = Makefile.in

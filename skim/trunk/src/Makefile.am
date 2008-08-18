#SUBDIRS = pluginbase
# set the include path for X, qt and KDE
#INCLUDES = $(all_includes)

# these are the headers for your project
libskimincludedir       = $(includedir)/skim
libskiminclude_HEADERS  = socketserverthread.h skimpluginmanager.h skimglobalactions.h skimplugin.h skimkeygrabber.h extra_utils.h
noinst_HEADERS = scimhelpdialog.h

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

autostartdir = $(datadir)/autostart
autostart_DATA = skim.desktop

#messages: 
#	$(XGETTEXT) *.cpp -o $(podir)/skim.pot

KDE_ICON = AUTO

#########################################################################
# APPLICATION SECTION
#########################################################################
# this is the program that gets installed.  it's name is used for all
# of the other Makefile.am variables
bin_SCRIPTS = scim-panel-kde
bin_PROGRAMS = skim

INCLUDES                = @SCIM_CFLAGS@ \
			  -I$(top_srcdir)/ \
			  -I$(top_builddir)/ \
			  $(all_includes) #should be kept
#-DSCIM_KDE_MODULE_ROOT_PATH=\"$(libdir)/skim/\" \
# the application source, library search path, and link libraries
skim_SOURCES = main.cpp
skim_LDFLAGS = $(all_libraries)
skim_LDADD = libskim.la

# this is where the desktop file will go 
shelldesktopdir   = $(kde_appsdir)/Utilities
shelldesktop_DATA = skim.desktop

# this is where the shell's XML-GUI resource file goes
#shellrcdir   = $(kde_datadir)/skim
#shellrc_DATA = skimui.rc

kde_servicetypes_DATA = skimplugin.desktop skimui.desktop skimkcmplugin.desktop skimsetupcategory.desktop

lib_LTLIBRARIES =  libskim.la
libskim_la_SOURCES = socketserverthread.cpp skimpluginmanager.cpp scimhelpdialog.cpp skimglobalactions.cpp skimplugin.cpp skimkeygrabber.cpp
libskim_la_LDFLAGS = -version-info $(SKIM_CURRENT):$(SKIM_REVISION):$(SKIM_AGE) \
	             $(KDE_RPATH) $(all_libraries) $(LIB_KDEUI) @SCIM_LIBS@ @SCIM_X11_LIBS@
libskim_la_LIBADD = $(top_builddir)/utils/libscim-kdeutils.la $(LIB_KUTILS)

DISTCLEANFILES = scim-panel-kde

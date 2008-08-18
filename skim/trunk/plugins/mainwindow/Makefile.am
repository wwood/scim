SUBDIRS=applet
noinst_HEADERS = mainwindow.h configmainwindow.h
METASOURCES = AUTO
INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/src $(all_includes) @SCIM_CFLAGS@
kde_module_LTLIBRARIES = skimplugin_mainwindow.la kcm_skimplugin_mainwindow.la
skimplugin_mainwindow_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries) -fPIC $(LIB_KDEUI)
skimplugin_mainwindow_la_SOURCES = mainwindow.cpp mainwindow.skel
skimplugin_mainwindow_la_LIBADD = $(top_builddir)/src/libskim.la $(top_builddir)/utils/libscim-kdeutils.la

#pkglib_LTLIBRARIES =  libmainwindow.la
#pkglibdir=$(libdir)/skim/plugins
#libmainwindow_ladir = $(pluginsdir)

#messages:
#	$(XGETTEXT) *.cpp -ki18n -o $(podir)/skim_ui.pot

shellrcdir   = $(kde_datadir)/skim
shellrc_DATA = mainwindowui.rc

kcm_skimplugin_mainwindow_la_LDFLAGS = -module -no-undefined $(KDE_PLUGIN) $(all_libraries)
kcm_skimplugin_mainwindow_la_SOURCES = mainwindow_settings.ui configmainwindow.cpp
kcm_skimplugin_mainwindow_la_LIBADD = $(LIB_KUTILS) $(top_builddir)/utils/libscim-kdeutils.la $(top_builddir)/src/libskim.la

kcm_DATA = skimplugin_mainwindow_config.desktop
kcmdir = $(kde_servicesdir)/skimconfiguredialog

# Install the desktop file needed to detect the plugin
kde_services_DATA = skimplugin_mainwindow.desktop
#kcmdir = $(kde_servicesdir)/kconfiguredialog

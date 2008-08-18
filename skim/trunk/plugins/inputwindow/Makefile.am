#SUBDIRS = setup_iw
#pkglib_LTLIBRARIES =  libinputwindow.la
#pkglibdir=$(libdir)/skim/plugins

noinst_HEADERS = scimlistbox.h scimlineedit.h scimstringlistitem.h scimlookuptable.h configinputwindow.h scimstringrender.h inputwindow.h
METASOURCES = AUTO
INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/src -I$(top_srcdir)/utils $(all_includes) @SCIM_CFLAGS@

kde_module_LTLIBRARIES = skimplugin_inputwindow.la kcm_skimplugin_inputwindow.la
skimplugin_inputwindow_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries) -fPIC $(LIB_KDEUI)
skimplugin_inputwindow_la_SOURCES = inputwindow.cpp scimlistbox.cpp scimlineedit.cpp scimstringlistitem.cpp scimlookuptable.cpp scimstringrender.cpp
skimplugin_inputwindow_la_LIBADD = $(top_builddir)/src/libskim.la $(top_builddir)/utils/libscim-kdeutils.la

kcm_skimplugin_inputwindow_la_LDFLAGS = -module -no-undefined $(KDE_PLUGIN) $(all_libraries)
kcm_skimplugin_inputwindow_la_SOURCES = inputwindow_settings.ui configinputwindow.cpp
kcm_skimplugin_inputwindow_la_LIBADD = $(LIB_KUTILS) $(top_builddir)/utils/libscim-kdeutils.la

kcm_DATA = skimplugin_inputwindow_config.desktop
kcmdir = $(kde_servicesdir)/skimconfiguredialog

# Install the desktop file needed to detect the plugin
kde_services_DATA = skimplugin_inputwindow.desktop
#kcmdir = $(kde_servicesdir)/kconfiguredialog

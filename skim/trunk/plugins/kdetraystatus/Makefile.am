noinst_HEADERS = kdetraystatus.h systemtrayplugin.h kcmloader.h
METASOURCES = AUTO
INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/src $(all_includes) @SCIM_CFLAGS@

kde_module_LTLIBRARIES = skimplugin_systemtray.la kcm_skimplugin_systemtray.la
skimplugin_systemtray_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries) -fPIC $(LIB_KDEUI)
skimplugin_systemtray_la_SOURCES = systemtrayplugin.cpp kdetraystatus.cpp
skimplugin_systemtray_la_LIBADD = $(top_builddir)/src/libskim.la $(top_builddir)/utils/libscim-kdeutils.la

kcm_skimplugin_systemtray_la_LDFLAGS = -module -no-undefined $(KDE_PLUGIN) $(all_libraries)
kcm_skimplugin_systemtray_la_SOURCES = kdetraystatus_config.ui kcmloader.cpp
kcm_skimplugin_systemtray_la_LIBADD = $(LIB_KUTILS) $(top_builddir)/utils/libscim-kdeutils.la

kcm_DATA = skimplugin_systemtray_config.desktop
kcmdir = $(kde_servicesdir)/skimconfiguredialog

# Install the desktop file needed to detect the plugin
kde_services_DATA = skimplugin_systemtray.desktop
#kcmdir = $(kde_servicesdir)/kconfiguredialog

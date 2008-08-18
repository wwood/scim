#pkglib_LTLIBRARIES =  libscimlauncher.la
#pkglibdir=$(libdir)/skim/plugins

if XCOMPOSITE_IS_AVAILABLE
KDE_MODULE_XCOMPOSITE=skimplugin_compmgrclient.la kcm_skimplugin_compmgrclient.la
kcm_DATA = skimplugin_compmgrclient_config.desktop
kcmdir = $(kde_servicesdir)/skimconfiguredialog

kde_services_DATA = skimplugin_compmgrclient.desktop
endif

noinst_HEADERS = compmgrclient.h compositemanagerconfig.h
METASOURCES = AUTO
INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/src $(all_includes) @SCIM_CFLAGS@

kde_module_LTLIBRARIES = $(KDE_MODULE_XCOMPOSITE)
skimplugin_compmgrclient_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries) $(LIB_KDEUI)
skimplugin_compmgrclient_la_SOURCES = compmgrclient.cpp compmgrclient.skel
skimplugin_compmgrclient_la_LIBADD = $(top_builddir)/src/libskim.la 

kcm_skimplugin_compmgrclient_la_LDFLAGS = -module -no-undefined $(KDE_PLUGIN) $(all_libraries)
kcm_skimplugin_compmgrclient_la_SOURCES = compositemanagerbase.ui \
	compositemanagerconfig.cpp
kcm_skimplugin_compmgrclient_la_LIBADD = $(LIB_KUTILS) $(top_builddir)/utils/libscim-kdeutils.la $(top_builddir)/src/libskim.la

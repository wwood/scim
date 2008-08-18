#pkglib_LTLIBRARIES =  libscimlauncher.la
#pkglibdir=$(libdir)/skim/plugins

noinst_HEADERS = scimlauncher.h
METASOURCES = AUTO
INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/src $(all_includes) @SCIM_CFLAGS@

kde_module_LTLIBRARIES = skimplugin_scim.la
skimplugin_scim_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries) -fPIC $(LIB_KDEUI)
skimplugin_scim_la_SOURCES = scimlauncher.cpp
skimplugin_scim_la_LIBADD = $(top_builddir)/src/libskim.la 

kde_services_DATA = skimplugin_scim.desktop

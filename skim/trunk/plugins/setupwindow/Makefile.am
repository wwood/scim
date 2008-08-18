INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/src $(all_includes) @SCIM_CFLAGS@ 
METASOURCES = AUTO
kde_module_LTLIBRARIES =  skimplugin_setupwindow.la kcm_skimplugin_scim_x.la kcm_skimplugin_scim_imengines.la kcm_skimplugin_configplugin.la kcm_skimplugin_scim_global.la

noinst_HEADERS = scimsetupwindow.h scimxsettings.h scimimenginesettings.h \
			configskimplugins.h scimglobalconfigplugin.h scimattachfilter.h

skimplugin_setupwindow_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries) -fPIC $(LIB_KDEUI) $(LIB_KUTILS)
skimplugin_setupwindow_la_SOURCES = scimsetupwindow.cpp
skimplugin_setupwindow_la_LIBADD = $(top_builddir)/src/libskim.la $(top_builddir)/utils/libscim-kdeutils.la

# Install the desktop file needed to detect the plugin
kde_services_DATA = skimplugin_setupwindow.desktop skimsetupdir_panel.desktop skimsetupdir_frontend.desktop skimsetupdir_imengines.desktop

kcm_skimplugin_scim_x_la_LDFLAGS = -module -no-undefined $(KDE_PLUGIN) $(all_libraries)
kcm_skimplugin_scim_x_la_SOURCES = scim_x.ui scimxsettings.cpp
kcm_skimplugin_scim_x_la_LIBADD = $(LIB_KUTILS) $(top_builddir)/utils/libscim-kdeutils.la $(top_builddir)/src/libskim.la

kcm_skimplugin_scim_imengines_la_LDFLAGS = -module -no-undefined $(KDE_PLUGIN) $(all_libraries)
kcm_skimplugin_scim_imengines_la_SOURCES = scimimenginesettings.cpp \
	scimimenginesettingsui.ui scimattachfilter.cpp attachfilteruibase.ui filterinfodlgbase.ui
kcm_skimplugin_scim_imengines_la_LIBADD = $(LIB_KUTILS) $(top_builddir)/utils/libscim-kdeutils.la $(top_builddir)/src/libskim.la

kcm_skimplugin_configplugin_la_LDFLAGS = -module -no-undefined $(KDE_PLUGIN) $(all_libraries)
kcm_skimplugin_configplugin_la_SOURCES = configskimplugins.cpp
kcm_skimplugin_configplugin_la_LIBADD = $(LIB_KUTILS) $(top_builddir)/utils/libscim-kdeutils.la $(top_builddir)/src/libskim.la

kcm_skimplugin_scim_global_la_LDFLAGS = -module -no-undefined $(KDE_PLUGIN) $(all_libraries)
kcm_skimplugin_scim_global_la_SOURCES = scim_global.ui scimglobalconfigplugin.cpp
kcm_skimplugin_scim_global_la_LIBADD = $(LIB_KUTILS) $(top_builddir)/utils/libscim-kdeutils.la $(top_builddir)/src/libskim.la

#kde_kcfg_DATA =

kcm_DATA = skimplugin_scim_x_config.desktop skimplugin_scim_imengines_config.desktop \
	   kcm_skimplugin_configplugin_config.desktop \
	   skimplugin_scim_global_config.desktop

kcmdir = $(kde_servicesdir)/skimconfiguredialog

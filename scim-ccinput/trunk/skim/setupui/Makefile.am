#prefix = @skim_prefix@
INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/src $(all_includes) @SCIM_KDEUTILS_CFLAGS@ 
METASOURCES = AUTO
kde_module_LTLIBRARIES =  kcm_skimplugin_scim_ccinput.la

kcm_skimplugin_scim_ccinput_la_LDFLAGS = -module -no-undefined $(KDE_PLUGIN) $(all_libraries)
kcm_skimplugin_scim_ccinput_la_SOURCES = ccinput.kcfgc ccinputui.ui scimccinputsettingplugin.cpp
kcm_skimplugin_scim_ccinput_la_LIBADD = $(LIB_QT) $(LIB_KDEUI) $(LIB_KDECORE) @SCIM_KDEUTILS_LIBS@

kde_kcfg_DATA = ccinput.kcfg

kcm_DATA = skimplugin_scim_ccinput_config.desktop

kcmdir = $(kde_servicesdir)/skimconfiguredialog

INCLUDES = -I$(top_srcdir)/src -I$(top_builddir)/src -I$(top_srcdir)/src $(all_includes)
METASOURCES = AUTO
libscim_kdeutils_la_LDFLAGS = -version-info $(SKIM_CURRENT):$(SKIM_REVISION):$(SKIM_AGE) $(all_libraries)
lib_LTLIBRARIES =  libscim-kdeutils.la
libscim_kdeutils_la_SOURCES = scimkdesettings.kcfgc scimmovehandle.cpp \
	scimdragableframe.cpp scimxmlguibuilder.cpp scimactions.cpp scimtoolbar.cpp kautocmodule.cpp \
	skimplugininfo.cpp
kde_kcfg_DATA = scimkdeconfig.kcfg

libscimkdeutilsincludedir       = $(includedir)/skim/utils
libscimkdeutilsinclude_HEADERS  = scimmovehandle.h scimdragableframe.h scimxmlguibuilder.h scimactions.h scimtoolbar.h kautocmodule.h
#noinst_HEADERS = scimmovehandle.h scimdragableframe.h scimxmlguibuilder.h scimactions.h scimtoolbar.h kautocmodule.h
noinst_HEADERS = skimplugininfo.h

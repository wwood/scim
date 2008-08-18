
INCLUDES = $(all_includes)

kde_module_LTLIBRARIES = skim_panelapplet.la

skim_panelapplet_la_SOURCES = skimapplet.skel skimapplet.cpp

skim_panelapplet_la_METASOURCES = AUTO
noinst_HEADERS = skimapplet.h

lnkdir = $(kde_datadir)/kicker/applets
lnk_DATA = skimapplet.desktop

EXTRA_DIST = $(lnk_DATA)

skim_panelapplet_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
skim_panelapplet_la_LIBADD = $(LIB_KDEUI) $(LIB_KIO)

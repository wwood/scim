if SCIM_BUILD_TESTS
CONFIG_TEST_PROGS = edittest
endif
noinst_PROGRAMS = $(CONFIG_TEST_PROGS)

edittest_SOURCES = testinputwidgets.ui main.cpp
edittest_LDADD = $(all_libraries) $(LIB_QT)
INCLUDES = $(all_includes)

#noinst_HEADERS =

METASOURCES = AUTO

KDE_OPTIONS = qtonly

MAINTAINERCLEANFILES = Makefile.in

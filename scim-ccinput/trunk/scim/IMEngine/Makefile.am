## Makefile.am -- Process this file with automake to produce Makefile.in
## Copyright (C) 2002 James Su
##
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2, or (at your option)
## any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software
## Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

MAINTAINERCLEANFILES    = Makefile.in
CLEANFILES              = *.bak

INCLUDES                = -I$(top_srcdir)/src @SCIM_CFLAGS@ \
			  -DSCIM_CCIN_SERVER_MODULE_DATADIR=\"@SCIM_DATADIR@/ccinput\"
			  -DSCIM_ICONDIR=\"@SCIM_ICONDIR@\"

noinst_HEADERS          = scim_generic_table.h \
                          scim_ccin_imengine.h

CONFIG_SERVER_CCIN_MODULE       = ccin.la
CONFIG_GENERIC_TABLE_LIB        = libgenerictable.la

noinst_LTLIBRARIES              = $(CONFIG_GENERIC_TABLE_LIB)
libgenerictable_la_SOURCES      = scim_generic_table.cpp

ccin_la_SOURCES        = scim_ccin_imengine.cpp

ccin_la_LDFLAGS        = -avoid-version -no-undefined\
                          -rpath $(moduledir) \
                          -module\
                          @LIBTOOL_EXPORT_OPTIONS@ \
                          @INTLLIBS@ \
                          @SCIM_LIBS@

ccin_la_LIBADD         = libgenerictable.la \
                         $(top_builddir)/src/libccinput.la

moduledir               = @SCIM_MODULEDIR@/IMEngine
module_LTLIBRARIES      = $(CONFIG_SERVER_CCIN_MODULE)

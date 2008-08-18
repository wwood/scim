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

MAINTAINERCLEANFILES	= Makefile.in
CLEANFILES		= *.bak

INCLUDES		= -I$(top_builddir) \
	    		  -I$(top_srcdir) \
			  -I$(top_srcdir)/src \
			  -I$(top_srcdir)/utils @SCIM_CFLAGS@ \
			  -I$(top_builddir) \
			  -I$(top_builddir)/utils \
			  $(all_includes)

pkglib_LTLIBRARIES =  kconfig.la
pkglibdir=$(SCIM_LIBDIR)/scim-1.0/$(SCIM_BINARY_VERSION)/Config

noinst_HEADERS		= scim_kconfig_config.h

kconfig_la_SOURCES 	= scim_kconfig_config.cpp
kconfig_la_LIBADD	= $(top_builddir)/utils/libscim-kdeutils.la @SCIM_LIBS@ $(LIB_KDEUI)
#$(top_builddir)/src/libscim.la

kconfig_la_LDFLAGS       = -avoid-version -module $(all_libraries) @LTLIBINTL@


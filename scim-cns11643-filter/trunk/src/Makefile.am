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
			  -I$(top_builddir)/src \
	    		  -I$(top_srcdir) \
			  -I$(top_srcdir)/src \
			  -I$(top_srcdir)/intl \
			  -I$(top_srcdir)/utils \
			  -DSCIM_DATADIR=\"@SCIM_DATADIR@\" \
			  -DSCIM_LOCALEDIR=\"@SCIM_LOCALEDIR@\" \
			  -DSCIM_SYSCONFDIR=\"@SCIM_SYSCONFDIR@\" \
			  -DSCIM_LIBEXECDIR=\"@SCIM_LIBEXECDIR@\" \
			  -DSCIM_ICONDIR=\"@SCIM_ICONDIR@\" \
			  -DSCIM_MODULE_PATH=\"@SCIM_MODULE_PATH@\" \
			  -DSCIM_TEMPDIR=\"@SCIM_TEMPDIR@\"

CONFIG_FILTER_CNS11643  = cns11643.la

noinst_HEADERS		= scim_cns11643_filter.h \
			  scim_cns11643_data.h

moduledir		= $(libdir)/scim-1.0/Filter
module_LTLIBRARIES	= $(CONFIG_FILTER_CNS11643)

cns11643_la_SOURCES 	= scim_cns11643_filter.cpp

cns11643_la_CXXFLAGS	= @SCIM_CFLAGS@
cns11643_la_LDFLAGS	= -avoid-version \
		     	  -rpath $(moduledir) \
			  -module \
			  @LTLIBINTL@ \
			  @SCIM_LIBS@


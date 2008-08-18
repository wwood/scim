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

INCLUDES		= @SCIM_GTKUTILS_CFLAGS@ \
			  -DSCIM_CCIN_LOCALEDIR=\"$(datadir)/locale\"

CONFIG_SERVER_CCIN_SETUP_MODULE= ccin-imengine-setup.la

# Rules for setup module
setupmoduledir           = @SCIM_MODULEDIR@/SetupUI

setupmodule_LTLIBRARIES  = $(CONFIG_SERVER_CCIN_SETUP_MODULE)

ccin_imengine_setup_la_SOURCES = scim_ccin_imengine_setup.cpp

ccin_imengine_setup_la_CFLAGS  = @SCIM_GTKUTILS_CFLAGS@ @SCIM_CFLAGS@

ccin_imengine_setup_la_LDFLAGS = -avoid-version \
			     	-rpath $(setupmoduledir) \
				-module \
				@LIBTOOL_EXPORT_OPTIONS@ \
				@INTLLIBS@ \
				@SCIM_LIBS@

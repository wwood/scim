## Makefile.am -- Process this file with automake to produce Makefile.in
##
## Copyright (C) 2006 Ryo Dairiki
##
##
## This library is free software; you can redistribute it and/or
## modify it under the terms of the GNU Lesser General Public
## License as published by the Free Software Foundation and 
## appearing in the file LICENSE.LGPL included in the package of this file.
## You can also redistribute it and/or modify it under the terms of 
## the GNU General Public License as published by the Free Software Foundation and 
## appearing in the file LICENSE.GPL included in the package of this file.
##
## This library is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

MAINTAINERCLEANFILES 	= Makefile.in

INCLUDES		= -I$(top_builddir) \
	    		  -I$(top_srcdir) \
			  -I$(top_srcdir)/common \
			  -I$(top_srcdir)/client-common

noinst_HEADERS = scim-bridge-client-gtk.h scim-bridge-client-imcontext-gtk.h  scim-bridge-client-key-event-utility-gtk.h

moduledir = @GTK_LIBDIR@/gtk-2.0/immodules

module_LTLIBRARIES = im-scim-bridge.la

im_scim_bridge_la_SOURCES = im-scim-bridge-gtk.c \
							scim-bridge-client-gtk.c \
							scim-bridge-client-imcontext-gtk.c \
							scim-bridge-client-key-event-utility-gtk.c

im_scim_bridge_la_CXXFLAGS=@GTK2_CFLAGS@
im_scim_bridge_la_CFLAGS  =@GTK2_CFLAGS@

im_scim_bridge_la_LDFLAGS = -rpath $(moduledir) \
		     -avoid-version -no-undefined \
		     -module \
		     @GTK2_LIBS@

im_scim_bridge_la_LIBADD  = $(top_srcdir)/common/libscimbridgecommon.la \
							$(top_srcdir)/client-common/libscimbridgeclientcommon.la

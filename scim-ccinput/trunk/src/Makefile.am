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

INCLUDES		= -I$(top_srcdir)/src 

CONFIG_SERVER_CCINPUT_MODULE	= libccinput.la

noinst_HEADERS		= datatype.h \
			  file_operation.h \
			  imcontext.h \
			  ccinput.h \
			  glossary_lookup.h \
			  glossary_adjust.h \
			  pinyin_parse.h 

#moduledir		= $(libdir)/
noinst_LTLIBRARIES	= $(CONFIG_SERVER_CCINPUT_MODULE)
		  
libccinput_la_SOURCES      = file_operation.c \
			  imcontext.c \
			  pinyin_table.c \
			  glossary_lookup.c \
			  glossary_adjust.c \
			  pinyin_parse.c 
        
#ccinput_la_LDFLAGS        = -avoid-version \
#                          -rpath $(moduledir) \
#                          -module\
#			  @LIBTOOL_EXPORT_OPTIONS@ \
#			  @INTLLIBS@                                                                                                                                             
#cc_la_LIBADD         = $(top_builddir)/src/libscim.la libgenerictable.la
#ccinput_la_LIBADD         = 


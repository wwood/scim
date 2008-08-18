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

INCLUDES		= -I$(top_builddir) \
	    		  -I$(top_srcdir) \
			  -I$(top_srcdir)/src \
			  -I$(top_srcdir)/intl \
			  -DSCIM_ICONDIR=\"@SCIM_ICONDIR@\" \
			  -DSCIM_PINYIN_LOCALEDIR=\"$(datadir)/locale\" \
			  -DSCIM_PINYIN_DATADIR=\"@SCIM_PINYIN_DATADIR@\" \
			  -DSCIM_SMART_PINYIN_ICON_FILE=\"@SCIM_ICONDIR@/smart-pinyin.png\"

noinst_HEADERS		= scim_pinyin_private.h \
			  scim_stl_map.h \
			  scim_phrase.h \
			  scim_pinyin.h \
		  	  scim_pinyin_global.h \
			  scim_pinyin_imengine.h \
			  scim_pinyin_imengine_config_keys.h \
			  scim_pinyin_phrase.h \
			  scim_native_lookup_table.h \
			  scim_pinyin_smart_match.h \
			  scim_special_table.h

noinst_LTLIBRARIES	= libpinyin.la

libpinyin_la_SOURCES	= scim_pinyin.cpp \
			  scim_phrase.cpp \
			  scim_pinyin_phrase.cpp \
			  scim_pinyin_global.cpp \
			  scim_pinyin_smart_match.cpp \
			  scim_special_table.cpp \
			  scim_native_lookup_table.cpp

libpinyin_la_CFLAGS	= @SCIM_CFLAGS@
libpinyin_la_CXXFLAGS	= @SCIM_CFLAGS@
libpinyin_la_LDFLAGS	= @SCIM_LIBS@ 

moduledir		= @SCIM_MODULEDIR@/IMEngine
module_LTLIBRARIES	= pinyin.la

pinyin_la_SOURCES 	= scim_pinyin_imengine.cpp

pinyin_la_CFLAGS	= @SCIM_CFLAGS@
pinyin_la_CXXFLAGS	= @SCIM_CFLAGS@

pinyin_la_LDFLAGS	= -avoid-version \
		     	  -rpath $(moduledir) \
			  -module \
			  @LIBTOOL_EXPORT_OPTIONS@ \
			  @SCIM_LIBS@ \
			  @LTLIBINTL@

pinyin_la_LIBADD	= libpinyin.la

if SCIM_BUILD_SETUP
SETUP_MODULE = pinyin-imengine-setup.la
endif

setupmoduledir           = @SCIM_MODULEDIR@/SetupUI
setupmodule_LTLIBRARIES  = $(SETUP_MODULE)

pinyin_imengine_setup_la_SOURCES = scim_pinyin_imengine_setup.cpp

pinyin_imengine_setup_la_CFLAGS  = @SCIM_GTKUTILS_CFLAGS@ @SCIM_CFLAGS@

pinyin_imengine_setup_la_CXXFLAGS= @SCIM_GTKUTILS_CFLAGS@ @SCIM_CFLAGS@

pinyin_imengine_setup_la_LDFLAGS = -avoid-version \
				 -rpath $(setupmoduledir) \
				 -module \
				 @LIBTOOL_EXPORT_OPTIONS@ \
				 @SCIM_LIBS@ \
				 @SCIM_GTKUTILS_LIBS@ \
			  	 @LTLIBINTL@

if ENABLE_TOOLS
TOOLS = make-pinyin make-phrase make-pinyin-phrase
endif

noinst_PROGRAMS		= $(TOOLS)

make_pinyin_SOURCES	= make_pinyin.cpp
make_pinyin_CXXFLAGS	= @SCIM_CFLAGS@
make_pinyin_LDFLAGS	= @SCIM_LIBS@
make_pinyin_LDADD	= libpinyin.la

make_phrase_SOURCES	= make_phrase.cpp
make_phrase_CXXFLAGS	= @SCIM_CFLAGS@
make_phrase_LDFLAGS	= @SCIM_LIBS@
make_phrase_LDADD	= libpinyin.la

make_pinyin_phrase_SOURCES	= make_pinyin_phrase.cpp
make_pinyin_phrase_CXXFLAGS	= @SCIM_CFLAGS@
make_pinyin_phrase_LDFLAGS	= @SCIM_LIBS@
make_pinyin_phrase_LDADD	= libpinyin.la

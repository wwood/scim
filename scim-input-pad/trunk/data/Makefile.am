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

icondir			= $(SCIM_ICONDIR)
icon_DATA		= input-pad.png

bin_SCRIPTS		= scim-input-pad

INPUT_PAD_SOURCE_FILES	= base.pad.in \
			  japanese.pad.in \
			  chinese.pad.in \
			  keyboard.pad.in

INPUT_PAD_DEST_FILES    = base.pad \
			  japanese.pad \
			  chinese.pad \
			  keyboard.pad

scim_input_pad_datadir	= @SCIM_INPUT_PAD_DATADIR@
scim_input_pad_data_DATA= $(INPUT_PAD_DEST_FILES)

EXTRA_DIST		= input-pad.png $(INPUT_PAD_SOURCE_FILES)

CLEANFILES		= *.bak scim-input-pad $(INPUT_PAD_DEST_FILES)

%.pad:   %.pad.in 	$(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*.po) ; LC_ALL=C $(INTLTOOL_MERGE) -d -u -c $(top_builddir)/po/.intltool-merge-cache $(top_srcdir)/po $< $@

MAINTAINERCLEANFILES 	= Makefile.in

INCLUDES		= -I$(top_builddir) \
	    		  -I$(top_srcdir) \
	    		  -I$(top_srcdir)/intl \
			  -I$(top_srcdir)/src \
			  -DSCIM_ICONDIR=\"@SCIM_ICONDIR@\" \
			  -DSCIM_INPUT_PAD_LOCALEDIR=\"$(datadir)/locale\" \
			  -DSCIM_INPUT_PAD_DATADIR=\"@SCIM_INPUT_PAD_DATADIR@\" \
			  -DSCIM_INPUT_PAD_USER_DATADIR=\"@SCIM_INPUT_PAD_USER_DATADIR@\"

noinst_HEADERS		= scim_input_group.h

lib_LTLIBRARIES 	= libscim-input-pad.la
libscim_input_pad_la_SOURCES = scim_input_group.cpp
libscim_input_pad_la_CXXFLAGS = @SCIM_CFLAGS@
libscim_input_pad_la_LDFLAGS = -version-info $(SCIM_INPUT_PAD_CURRENT):$(SCIM_INPUT_PAD_REVISION):$(SCIM_INPUT_PAD_AGE) \
			  -export-dynamic \
			  -rpath $(libdir) \
			  @LIBTOOL_EXPORT_OPTIONS@ \
			  @LTLIBINTL@ \
			  @SCIM_LIBS@

moduledir		= @SCIM_MODULEDIR@/Helper
module_LTLIBRARIES	= input-pad.la
input_pad_la_SOURCES	= scim_input_pad.cpp

input_pad_la_CXXFLAGS	= @GTK2_CFLAGS@ @SCIM_CFLAGS@
input_pad_la_LDFLAGS	= -avoid-version \
		     	  -rpath $(moduledir) \
			  -module \
			  @LIBTOOL_EXPORT_OPTIONS@ \
			  @LTLIBINTL@ \
			  @GTK2_LIBS@

input_pad_la_LIBADD	= libscim-input-pad.la

KDE_DOCS = skim
KDE_LANG = de

KDE_NOCHUNK_STYLESHEET="/usr/kde/3.3/share/apps/ksgmltools2/customization/kde-nochunk.xsl"
web: $(srcdir)/index.docbook $(KDE_XSL_STYLESHEET)
	@if test -n "$(MEINPROC)"; then echo $(MEINPROC) --stylesheet $(KDE_NOCHUNK_STYLESHEET) $(srcdir)/index.docbook; $(MEINPROC) --stylesheet $(KDE_NOCHUNK_STYLESHEET) $(srcdir)/index.docbook; fi

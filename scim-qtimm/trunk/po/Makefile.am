POFILES = AUTO
messages:
	find $(top_srcdir) -type d | grep -v CVS | grep -v skim | sed -e 's,$$,/,' > dirs #ignore skim dir too
	mfs=`find $(top_srcdir) -name Makefile.am | xargs egrep -l '^messages:'` ;\
	for dir in $$mfs; do \
	  if test ! "$$dir" = "./Makefile.am"; then \
	    dir=`dirname $$dir` ;\
	    egrep -v "^$$dir" dirs > dirs.new && mv dirs.new dirs ;\
	  fi ;\
	done
	egrep -v "/tests" dirs > dirs.new && mv dirs.new dirs ;\
	dirs=`cat dirs` ;\
	find $$dirs -maxdepth 1 -name "*.cpp" -print > files ;\
	find $$dirs -maxdepth 1 -name "*.cc" -print >> files ;\
	find $$dirs -maxdepth 1 -name "*.h" -print >> files ; \
	if test -z "$$EXTRACTRC"; then \
	  EXTRACTRC=extractrc ;\
	fi;\
	$$EXTRACTRC $(top_srcdir)/*/*.rc > rc.cpp
	$(XGETTEXT)  rc.cpp `cat files` -k_ -ktr2i18n -ki18n -o $(top_srcdir)/po/$(PACKAGE).pot
	rm -f rc.cpp dirs files
update-all: messages merge
MAINTAINERCLEANFILES = Makefile.in
#merge:
#	$(MAKE) -f $(top_srcdir)/admin/Makefile.common package-merge POFILES="${POFILES}" PACKAGE=${PACKAGE}
#	$(EXTRACTRC) `find $$dirs -maxdepth 1 -name "*.rc" -o -name "*.ui"` > rc.cpp

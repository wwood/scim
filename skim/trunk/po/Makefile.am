POFILES = AUTO
PACKAGE="skim"
XGETTEXT="xgettext"
GMSGFMT="gmsgfmt"
top_srcdir=..

apply: apply.cc
	@g++ apply.cc -o apply
desktops: apply
	if tempfile > /dev/null 2>&1; then \
		alias mytempfile=tempfile ;\
	else \
		alias mytempfile="echo -n /tmp/;uuidgen" ;\
	fi ;\
	langfile=`mytempfile` ;\
	rm -f $$langfile ;\
	KDEDIR=`mytempfile` ;\
	if test -f $$KDEDIR; then rm -f $$KDEDIR; mkdir $$KDEDIR; fi ;\
	export KDEDIR ;\
        files=`find $(top_srcdir)/po -name "desktop_*.po"` ;\
	for file in $$files; do \
		charset=`grep charset= $$file | sed -e "s#^.*[cC]harset=\(.*\)..\"#\1#"` ;\
		if test ! "x$$charset" = "xutf8" && test ! "x$$charset" = "xutf-8" && test ! "x$$charset" = "xUTF-8"; then\
			continue;\
		fi;\
		lang=`echo $$file | sed -e "s/.*desktop_\([^/\.]*\)\.po/\1/"`;\
		mkdir -p $$KDEDIR/share/locale/$$lang/LC_MESSAGES;\
		echo "Generate gmo file for language: $$lang";\
		$(GMSGFMT) -c --statistics $$file -o $$KDEDIR/share/locale/$$lang/LC_MESSAGES/apply.mo;\
		echo $$lang >> $$langfile;\
	done;\
	list=`cat $(top_srcdir)/po/desktop_file_list`;\
	for i in $$list; do\
	   ./apply ../$$i `sort -u $$langfile` >> logfile 2>&1 && \
	   echo -n "Patching file ../$$i ... " && \
	   if diff ../$$i ../$$i.new > /dev/null; then rm ../$$i.new; echo "";\
	   else mv -f ../$$i.new ../$$i; echo "UPDATED"; fi;\
	done;\
	sort logfile;\
	rm -f logfile $$langfile;\
	rm -rf $$KDEDIR

clean-local:
	rm -rf apply all_files* messages

messages:
	@echo "Generating/Updating Messages" ;\
	echo "Generating/Updating Source Messages" ;\
	find $(top_srcdir) -type d | grep -v CVS | sed -e 's,$$,/,' > dirs ;\
	mfs=`find $(top_srcdir) -name Makefile.am | xargs egrep -l '^messages:'` ;\
	for dir in $$mfs; do \
	  if test ! "$$dir" = "./Makefile.am"; then \
	    dir=`dirname $$dir` ;\
	    egrep -v "^$$dir" dirs > dirs.new && mv dirs.new dirs ;\
	  fi ;\
	done;\
	egrep -v "/tests" dirs > dirs.new && mv dirs.new dirs ;\
	dirs=`cat dirs` ;\
	find $$dirs -maxdepth 1 -name "*.cpp" -print > files ;\
	find $$dirs -maxdepth 1 -name "*.cc" -print >> files ;\
	find $$dirs -maxdepth 1 -name "*.h" -print >> files ; \
	if test -z "$$EXTRACTRC"; then \
	  EXTRACTRC=extractrc ;\
	fi;\
	$$EXTRACTRC $(top_srcdir)/*/*.rc > rc.cpp ;\
	$$EXTRACTRC $(top_srcdir)/*/*/*.rc >> rc.cpp ;\
	echo "Files considered:";\
	cat files;\
	$(XGETTEXT)  rc.cpp `cat files` -ktr2i18n -ki18n -kI18N_NOOP -o $(top_srcdir)/po/$(PACKAGE).pot ;\
	rm -f rc.cpp dirs files ;\
	echo "Generating/Updating Desktop File Messages" ;\
	cd .. && ./po/createdesktop ./po/desktop_file_list ./po/desktop.pot && cd po

merge:
	for cat in `ls *.po`; do \
  if test "x$${cat:0:7}" = "xdesktop"; then \
    msgmerge -o $${cat}.new $$cat desktop.pot; \
  else \
    msgmerge -o $${cat}.new $$cat ${PACKAGE}.pot; \
  fi; \
  if test -s $$cat.new; then \
    grep -v "\"POT-Creation" $$cat.new > $$cat.new.2; \
    grep -v "\"POT-Creation" $$cat >> $$cat.new.1; \
    if diff $$cat.new.1 $$cat.new.2; then \
        rm $$cat.new; \
    else \
        mv $$cat.new $$cat; \
    fi; \
    rm -f $$cat.new.1 $$cat.new.2; \
  fi; \
done
update-all: messages merge desktops
#	$(EXTRACTRC) `find $$dirs -maxdepth 1 -name "*.rc" -o -name "*.ui"` > rc.cpp

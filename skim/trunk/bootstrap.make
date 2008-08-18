#~/bin/bash
gmake -f admin/Makefile.common cvs
#gmake -f Makefile.cvs
if test "$1" = "release"; then
  echo "Cleaning files"
  find . -name .cvsignore | xargs rm -fr
  rm templates/ -fr
  find -name \CVS | xargs rm -rf
  rm autom4te.cache scim_panel_kde.kdevelop stamp*.in -fr
fi
#set -x
#aclocal-1.8
#autoheader
#libtoolize -c --automake
#intltoolize -c --automake
#automake-1.8 --add-missing --copy --include-deps
#perl -w admin/am_edit
#autoconf


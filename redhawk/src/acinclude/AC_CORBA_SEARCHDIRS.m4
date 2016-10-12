dnl
dnl  AC_CORBA_SEARCHDIRS( DIRTYPE )
dnl
dnl Description
dnl 
dnl  Utility macro. Constructs a space-separated list of directories to search
dnl  for CORBA components. Sets the variable `ac_corba_searchdirs'.
dnl  Example: `AC_CORBA_SEARCHDIRS([bin])' sets
dnl    ac_corba_searchdirs="$prefix/bin /usr/local/bin /opt/bin /usr/bin"
dnl  or
dnl    ac_corba_searchdirs="$OMNIORBBASE/bin"
dnl  If OMNIORBBASE is set (e.g. by --with-omniorb) then no other directories
dnl  are searched.
dnl 
dnl Copyright (C) 2003, Alex Tingle <alex.autoconf@firetree.net>
dnl 
dnl License:
dnl GNU General Public License
dnl [http://www.gnu.org/software/ac-archive/htmldoc/COPYING.html]
dnl with this special exception
dnl [http://www.gnu.org/software/ac-archive/htmldoc/COPYING-Exception.html]. 
dnl 

AC_DEFUN([AC_CORBA_SEARCHDIRS],[
  ac_corba_searchdirs=""
  if test "x$OMNIORBBASE" != x; then
    if test -d "$OMNIORBBASE/$1"; then
      ac_corba_searchdirs="$ac_corba_searchdirs $OMNIORBBASE/$1"
    fi
  else
    if test "x$prefix" != x && test "$prefix" != "NONE" && test -d "$prefix/$1"
    then
      ac_corba_searchdirs="$ac_corba_searchdirs $prefix/$1"
    fi
    if test -d "/usr/local/$1"
    then
      ac_corba_searchdirs="$ac_corba_searchdirs /usr/local/$1"
    fi
    if test -d "/opt/$1"
    then
      ac_corba_searchdirs="$ac_corba_searchdirs /opt/$1"
    fi
    if test -d "/usr/$1"
    then
      ac_corba_searchdirs="$ac_corba_searchdirs /usr/$1"
    fi
  fi
])

dnl
dnl  AC_CORBA_IDLCOS
dnl
dnl Description
dnl 
dnl  Searches for the directory containing COS/CosNaming.idl
dnl  and records it in the output variable IDL_COS_DIR.
dnl
dnl Copyright (C) 2003, Alex Tingle <alex.autoconf@firetree.net>
dnl 
dnl License:
dnl GNU General Public License
dnl [http://www.gnu.org/software/ac-archive/htmldoc/COPYING.html]
dnl with this special exception
dnl [http://www.gnu.org/software/ac-archive/htmldoc/COPYING-Exception.html]. 
dnl 

AC_DEFUN([AC_CORBA_IDLCOS],[
  AC_REQUIRE([AC_PROG_OMNIIDL])
  AC_CACHE_CHECK([for IDL COS include directory],
    ac_cv_corba_idlcos,
    [ ac_cv_corba_idlcos=no
      if test "x$OMNIORBBASE" != x; then
        if test -d "$OMNIORBBASE/share/idl"; then
          ac_corba_idldirs="$ac_corba_idldirs $OMNIORBBASE/share/idl"
        fi
        if test -d "$OMNIORBBASE/idl"; then
          ac_corba_idldirs="$ac_corba_idldirs $OMNIORBBASE/idl"
        fi
      else
        if test "x$prefix" != x && test "$prefix" != "NONE"; then
          if test -d "$prefix/share/idl"; then
            ac_corba_idldirs="$ac_corba_idldirs $prefix/share/idl"
          fi
          if test -d "$prefix/idl"; then
            ac_corba_idldirs="$ac_corba_idldirs $prefix/idl"
          fi
        fi
        if test -d "/usr/local/share/idl"; then
          ac_corba_idldirs="$ac_corba_idldirs /usr/local/share/idl"
        fi
        if test -d "/usr/share/idl"; then
          ac_corba_idldirs="$ac_corba_idldirs /usr/share/idl"
        fi
        if test -d "/opt/share/idl"; then
          ac_corba_idldirs="$ac_corba_idldirs /opt/share/idl"
        fi
      fi
      if test -n "$ac_corba_idldirs"; then
        for ac_corba_i in `find $ac_corba_idldirs -type d -name COS 2>/dev/null`
        do
          if test -f "$ac_corba_i/CosNaming.idl"
          then
            ac_cv_corba_idlcos=`AS_DIRNAME(["$ac_corba_i"])`
            break
          fi
        done
      fi
    ])
  if test "x$ac_cv_corba_idlcos" != xno; then
    IDL_COS_DIR=$ac_cv_corba_idlcos
    AC_SUBST(IDL_COS_DIR)
  fi
])

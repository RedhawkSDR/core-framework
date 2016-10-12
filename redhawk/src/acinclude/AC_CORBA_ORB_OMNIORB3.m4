dnl
dnl  AC_CORBA_ORB_OMNIORB3
dnl
dnl Description
dnl 
dnl  Tests for a linkable installation of omniORB3
dnl  [http://omniorb.sourceforge.net]. If found, it defines
dnl  pre-processor macro `HAVE_OMNIORB4' and sets variables CPPFLAGS,
dnl  LIBS & LDFLAGS. Sets pthread & socket options if necessary.
dnl
dnl Copyright (C) 2003, Alex Tingle <alex.autoconf@firetree.net>
dnl 
dnl License:
dnl GNU General Public License
dnl [http://www.gnu.org/software/ac-archive/htmldoc/COPYING.html]
dnl with this special exception
dnl [http://www.gnu.org/software/ac-archive/htmldoc/COPYING-Exception.html]. 
dnl 

AC_DEFUN([AC_CORBA_ORB_OMNIORB3],[
  AC_REQUIRE([AC_CORBA_SOCKET_NSL])
  AC_REQUIRE([ACX_PTHREAD])
  AC_REQUIRE([AC_CORBA_OMNI_PLATFORM])
  AC_CACHE_CHECK([for omniORB3],
    ac_cv_corba_omniorb3,
    [ AC_LANG_SAVE
      AC_LANG_CPLUSPLUS
      AC_CXX_CLEAN_TEMPLATE_REPOSITORY

      # Save CPPFLAGS, LDFLAGS & LIBS
      ac_corba_save_cppflags="$CPPFLAGS"
      ac_corba_save_ldflags="$LDFLAGS"
      ac_corba_save_libs="$LIBS"
      LDFLAGS="$ac_corba_save_ldflags $PTHREAD_CFLAGS $PTHREAD_LIBS"
      CPPFLAGS="$ac_corba_save_cppflags $PTHREAD_CFLAGS"

      # Try to find the omniORB3 header file.Start with $OMNIORBBASE,
      # and then $prefix, else just try the default include path.
      ac_cv_corba_omniorb3=no
      AC_CORBA_SEARCHDIRS(include)
      for ac_corba_i in `find $ac_corba_searchdirs -type d -name omniORB3 2>/dev/null`
      do
        if test -f $ac_corba_i/CORBA.h
        then
          ac_corba_omnidir=`AS_DIRNAME(["$ac_corba_i"])`
          CPPFLAGS="$CPPFLAGS -I$ac_corba_omnidir"
          AC_TRY_CPP([#include <omniORB3/CORBA.h>],
            [ac_cv_corba_omniorb3="$ac_corba_omnidir"],
            [CPPFLAGS="$ac_corba_save_cppflags $PTHREAD_CFLAGS"])
        fi
        test "$ac_cv_corba_omniorb3" != no && break
      done
      if test "$ac_cv_corba_omniorb3" = no && test -z "$OMNIORBBASE"; then
        AC_TRY_CPP([#include <omniORB3/CORBA.h>],[ac_cv_corba_omniorb3=yes])
      fi

      # Try to find the omniORB3 libraries.
      if test "$ac_cv_corba_omniorb3" != no; then
        LIBS="$LIBS -lomniORB3 -lomniGK_stub -lomniDynamic3 -lomniGK_alone"
        LIBS="$LIBS -lomnithread"
        ac_corba_links=no
        AC_CORBA_SEARCHDIRS(lib)
        for ac_corba_i in `find $ac_corba_searchdirs -type f -name 'libomniORB3*' 2>/dev/null`
        do
          # Could check for all required libraries here.
          ac_corba_omnidir=`AS_DIRNAME(["$ac_corba_i"])`
          LDFLAGS="$LDFLAGS -L$ac_corba_omnidir"
          # Try to link.
          AC_TRY_LINK([#include <omniORB3/CORBA.h>],[CORBA::ORB_var orb],
            [ac_corba_links=yes],
            [LDFLAGS="$ac_corba_save_ldflags $PTHREAD_CFLAGS $PTHREAD_LIBS"])
          test "$ac_corba_links" = yes && break
        done
        if test "$ac_corba_links" = no; then
          AC_TRY_LINK([#include <omniORB3/CORBA.h>],[CORBA::ORB_var orb],
            [ac_corba_links=yes])
        fi
        test "$ac_corba_links" = no && ac_cv_corba_omniorb3=no
      fi

      if test "$ac_cv_corba_omniorb3" = no
      then
        # Restore CPPFLAGS LDFLAGS & LIBS
        CPPFLAGS="$ac_corba_save_cppflags"
        LDFLAGS="$ac_corba_save_ldflags"
        LIBS="$ac_corba_save_libs"
      fi
      AC_LANG_RESTORE
    ])
  if test "$ac_cv_corba_omniorb3" != no
  then
    CORBA_ORB="omniORB3"
    AC_SUBST(CORBA_ORB)
    AC_DEFINE(HAVE_OMNIORB3,1,"define if omniORB3 is available.")
    # Since we've found `omniORB', we'll need `omniidl'.
    AC_PROG_OMNIIDL
  fi
])

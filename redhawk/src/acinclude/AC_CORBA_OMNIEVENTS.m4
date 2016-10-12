dnl
dnl  AC_CORBA_OMNIEVENTS
dnl
dnl Description
dnl 
dnl  Tests for a linkable installation of omniEvents
dnl  [http://www.omnievents.org/]. If found, it defines
dnl  pre-processor macro `HAVE_OMNIEVENTS'.
dnl
dnl Copyright (C) 2004, Alex Tingle <alex.autoconf@firetree.net>
dnl 
dnl License:
dnl GNU General Public License
dnl [http://www.gnu.org/software/ac-archive/htmldoc/COPYING.html]
dnl with this special exception
dnl [http://www.gnu.org/software/ac-archive/htmldoc/COPYING-Exception.html]. 
dnl 

AC_DEFUN([AC_CORBA_OMNIEVENTS],[
  AC_REQUIRE([AC_CORBA_ORB])
  AC_CACHE_CHECK([for omniEvents],
    ac_cv_corba_omnievents,
    [ AC_LANG_SAVE
      AC_LANG_CPLUSPLUS
      AC_CXX_CLEAN_TEMPLATE_REPOSITORY

      # Save CPPFLAGS, LDFLAGS & LIBS
      ac_corba_save_cppflags="$CPPFLAGS"
      ac_corba_save_ldflags="$LDFLAGS"
      ac_corba_save_libs="$LIBS"
      LIBS="$ac_corba_save_libs -lomniEvents"
      # Nasty hack to get around problems with omniEvents 2.4.1 install.
      CPPFLAGS="$ac_corba_save_cppflags -I$ac_omniorbbase/include/COS"

      ac_cv_corba_omnievents=no
      AC_TRY_LINK(
        [#include <EventChannelAdmin.hh>],
        [EventChannelAdmin::EventChannelFactory_var factory],
        [ac_cv_corba_omnievents=yes]
      )

      if test "$ac_cv_corba_omnievents" = no
      then
        # Restore CPPFLAGS LDFLAGS & LIBS
        CPPFLAGS="$ac_corba_save_cppflags"
        LDFLAGS="$ac_corba_save_ldflags"
        LIBS="$ac_corba_save_libs"
      fi
      AC_LANG_RESTORE
    ])
  if test "$ac_cv_corba_omnievents" != no
  then
    AC_DEFINE(HAVE_OMNIEVENTS,1,"define if omniEvents is available.")
  fi
])

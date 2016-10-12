dnl
dnl AC_FUNC_SIGSET
dnl 
dnl Description
dnl 
dnl Tests for sigset(), the non-BSD alternative to signal().
dnl
dnl Copyright (C) 2004, Alex Tingle <alex.autoconf@firetree.net>
dnl 
dnl License:
dnl GNU General Public License
dnl [http://www.gnu.org/software/ac-archive/htmldoc/COPYING.html]
dnl with this special exception
dnl [http://www.gnu.org/software/ac-archive/htmldoc/COPYING-Exception.html]. 
dnl 


AC_DEFUN([AC_FUNC_SIGSET],[
  AC_CACHE_CHECK([for sigset()],[ac_cv_sigset],[
    AC_TRY_COMPILE([#include <signal.h>],[sigset(SIGTERM,SIG_IGN);],
      [ac_cv_sigset=yes], [ac_cv_sigset=no])
  ])
  if test x"ac_cv_sigset" = xyes; then
    AC_DEFINE([HAVE_SIGSET],1,[Define to 1 if you have the `sigset' function.])
  fi
])

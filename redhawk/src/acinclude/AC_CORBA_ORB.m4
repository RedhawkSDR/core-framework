dnl
dnl  AC_CORBA_ORB
dnl
dnl Description
dnl 
dnl  Tests for a linkable CORBA ORB. Currentlly only finds omniORB3 or
dnl  omniORB4. Sets the output variable `CORBA_ORB', sets variables CPPFLAGS,
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

AC_DEFUN([AC_CORBA_ORB],[
dnl   AC_REQUIRE(PKG_CHECK_EXISTS)
  if test "x$CORBA_ORB" = x; then
      AC_CACHE_CHECK([for omniORB4],
	ac_cv_corba_omniorb4,
        [PKG_CHECK_EXISTS([omniORB4 >= 4.0.0], [ac_cv_corba_omniorb4=yes], [ac_cv_corba_omniorb4=no])])

      if test "x$ac_cv_corba_omniorb4" = "xyes"; then
          AC_DEFINE([HAVE_OMNIORB4],1,"define if omniORB4 is available.")
          CORBA_ORB="omniORB4"
          AC_SUBST([CORBA_ORB])
      fi
  fi
  if test "x$CORBA_ORB" = x; then
      AC_CACHE_CHECK([for omniORB3],
	ac_cv_corba_omniorb3,
        [PKG_CHECK_EXISTS([omniORB3 >= 3.0.0], [ac_cv_corba_omniorb3=yes], [ac_cv_corba_omniorb3=no])])

      if test "x$ac_cv_corba_omniorb3" = "xyes"; then
          AC_DEFINE([HAVE_OMNIORB3],1,"define if omniORB3 is available.")
          CORBA_ORB="omniORB3"
          AC_SUBST([CORBA_ORB])
      fi
  fi
])


dnl
dnl AC_CORBA_SOCKET_NSL
dnl Small wrapper around ETR_SOCKET_NSL. Automatically adds the result to LIBS.
dnl 

AC_DEFUN([AC_CORBA_SOCKET_NSL],[
  AC_REQUIRE([AC_PROG_CC])
  AC_REQUIRE([ETR_SOCKET_NSL])
  if test "x$ETR_SOCKET_LIBS" != x; then
    LIBS="$LIBS $ETR_SOCKET_LIBS"
  fi
])

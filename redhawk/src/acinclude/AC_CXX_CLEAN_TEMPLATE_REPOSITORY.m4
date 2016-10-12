dnl 
dnl AC_CXX_CLEAN_TEMPLATE_REPOSITORY
dnl
dnl Description
dnl 
dnl Sometime failed C++ compiles can leave trash in the template repository.
dnl Just clean them all away.
dnl 
dnl Copyright (C) 2003, Alex Tingle <alex.autoconf@firetree.net>
dnl 
dnl License:
dnl GNU General Public License
dnl [http://www.gnu.org/software/ac-archive/htmldoc/COPYING.html]
dnl with this special exception
dnl [http://www.gnu.org/software/ac-archive/htmldoc/COPYING-Exception.html]. 
dnl 

AC_DEFUN([AC_CXX_CLEAN_TEMPLATE_REPOSITORY],[
  # Sometime failed C++ compiles can leave trash in the template repository.
  # Just clean them all away.
  rm -rf ./SunWS_cache # Solaris/CC
  rm -rf ./.cxx_repository # Tru64/cxx
  # ...add more here.
])

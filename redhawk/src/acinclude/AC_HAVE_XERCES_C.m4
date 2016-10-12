dnl
dnl This file is protected by Copyright. Please refer to the COPYRIGHT file 
dnl distributed with this source distribution.
dnl 
dnl This file is part of REDHAWK core.
dnl 
dnl REDHAWK core is free software: you can redistribute it and/or modify it under 
dnl the terms of the GNU Lesser General Public License as published by the Free 
dnl Software Foundation, either version 3 of the License, or (at your option) any 
dnl later version.
dnl 
dnl REDHAWK core is distributed in the hope that it will be useful, but WITHOUT 
dnl ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
dnl FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
dnl details.
dnl 
dnl You should have received a copy of the GNU Lesser General Public License 
dnl along with this program.  If not, see http://www.gnu.org/licenses/.
dnl

AC_DEFUN([AC_HAVE_XERCES_C], [

AC_ARG_WITH(xerces-prefix, [  --with-xerces-prefix=PFX   Prefix where
Xerces-C is installed (optional)],
            [xerces_prefix="$withval"], [xerces_prefix=""])

AC_LANG_PUSH([C++])
AC_MSG_CHECKING([for Xerces-C])

if test -n "$xerces_prefix" ; then
  CXXFLAGS="$CXXFLAGS -I$xerces_prefix/include"
  LDFLAGS="$LDFLAGS -L$xerces_prefix/lib"
fi

LIBS="-lxerces-c $LIBS"

AC_LINK_IFELSE([
 AC_LANG_PROGRAM(
  [
  #include <xercesc/parsers/XercesDOMParser.hpp>
  #include <iostream>
  XERCES_CPP_NAMESPACE_USE
  ],
  [
  XercesDOMParser* parser = new XercesDOMParser();
  ])],
 [AC_MSG_RESULT([yes])],
 [AC_MSG_FAILURE([missing])

])

AC_LANG_POP
])




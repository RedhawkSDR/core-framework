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

dnl Get the IDL path from a package
AC_DEFUN([RH_PKG_IDLDIR],
[
  AC_REQUIRE([PKG_PROG_PKG_CONFIG])

  AC_MSG_CHECKING([for $1 IDL path])
  $1[]_IDLDIR=`$PKG_CONFIG --variable=idldir "$2" 2>/dev/null`
  AC_MSG_RESULT($[$1[]_IDLDIR])
  AC_SUBST($1[]_IDLDIR)
])

dnl Get the Java CLASSPATH from a package
AC_DEFUN([RH_PKG_CLASSPATH],
[
  AC_REQUIRE([PKG_PROG_PKG_CONFIG])

  AC_MSG_CHECKING([for $1 classpath])
  PKG_CHECK_EXISTS([$2],
                   [$1[]_CLASSPATH=`$PKG_CONFIG --variable=classpath "$2" 2>/dev/null`
                    AC_MSG_RESULT([yes])],
	           [AC_MSG_RESULT([no])])
  AC_SUBST($1[]_CLASSPATH)
])

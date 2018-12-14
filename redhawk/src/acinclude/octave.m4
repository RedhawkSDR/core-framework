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

# RH_PROG_OCTAVE_CONFIG
# ---------------------
AC_DEFUN([RH_PROG_OCTAVE_CONFIG],
[
  AC_ARG_VAR([OCTAVE_CONFIG], [path to octave-config utility])
  AS_IF([test "x$ac_cv_env_OCTAVE_CONFIG_set" != "xset"], [
    AC_PATH_TOOL([OCTAVE_CONFIG], [octave-config])
  ])
])

# RH_OCTAVE([MINIMUM-VERSION])
#
# Checks for Octave installation, with an optional minimum version
# -----------------------------------------------------------------------------
AC_DEFUN([RH_OCTAVE],
[
  dnl Require octave-config to be able to get the include and library
  dnl directories
  AC_REQUIRE([RH_PROG_OCTAVE_CONFIG])
  AS_IF([test "x$OCTAVE_CONFIG" == "x"], [
    AC_ERROR([octave-config was not found])
  ])

  dnl If a minimum version was given, get the Octave version from octave-config
  dnl and compare; otherwise, just assume it should work
  AS_IF([test x$1 != x], [
    AC_MSG_CHECKING([for Octave >= $1])
    rh_octave_version=`$OCTAVE_CONFIG -v`
    AS_VERSION_COMPARE([$rh_octave_version], [$1], [
      AC_ERROR([Octave version $rh_octave_version found, $1 required])
    ], [], [])
  ], [
    AC_MSG_CHECKING([for Octave])
  ])

  dnl Get the include directory from octave-config, then format it into usable
  dnl include paths
  rh_octave_incdir=`$OCTAVE_CONFIG -p OCTINCLUDEDIR`
  OCTAVE_CPPFLAGS="-I${rh_octave_incdir}/.. -I${rh_octave_incdir}"
  AC_SUBST([OCTAVE_CPPFLAGS])

  dnl Get the library directory from octave-config for use as a linker path,
  dnl then add the "octave" an "octinterp" libraries to linker flags
  rh_octave_libdir=`$OCTAVE_CONFIG -p OCTLIBDIR`
  OCTAVE_LIBS="-L${rh_octave_libdir} -loctave -loctinterp"
  AC_SUBST([OCTAVE_LIBS])

  AC_MSG_RESULT([yes])
])

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

AC_DEFUN([AC_CHECK_PYMODULE],
[
AS_LITERAL_IF([$1], 
              [AS_VAR_PUSHDEF([ac_Pymod], [ac_cv_pymod_$1])],
              [AS_VAR_PUSHDEF([ac_Pymod], [ac_cv_pymod_$1''])])
AC_CACHE_CHECK(for python module $1, ac_Pymod,
  [
  $PYTHON -c "import $1" > /dev/null 2>&1
  ac_status=$?
  if (exit $ac_status); then
    ac_Pymod='yes'
  else
    ac_Pymod='no'
  fi
  ])
  AS_IF([test AS_VAR_GET(ac_Pymod) = yes], [$2], [$3])[]
  AS_VAR_POPDEF([ac_Pymod])
])

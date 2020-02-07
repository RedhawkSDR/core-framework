/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK GPP.
 *
 * REDHAWK GPP is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK GPP is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */
AC_INIT(GPP, 2.2.6)
AM_INIT_AUTOMAKE([foreign nostdinc subdir-objects])
AC_CONFIG_MACRO_DIR([m4])

AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL

AC_CORBA_ORB
OSSIE_CHECK_OSSIE
OSSIE_SDRROOT_AS_PREFIX

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

# Dependencies
export PKG_CONFIG_PATH="$PKG_CONFIG_PATH:/usr/local/lib/pkgconfig"
PKG_CHECK_MODULES([PROJECTDEPS], [ossie >= 2.2 omniORB4 >= 4.1.0  ])
OSSIE_ENABLE_LOG4CXX
AX_BOOST_BASE([1.41])
AX_BOOST_SYSTEM
AX_BOOST_THREAD
AX_BOOST_REGEX
AX_BOOST_FILESYSTEM

AC_ARG_ENABLE([affinity], AS_HELP_STRING([--disable-affinity], [Disable affinity processing]))
   AC_MSG_CHECKING([Adding support for affinity processing])
if test "x$enable_affinity" != "xno"; then
   dnl  determine if numa library support is available
   AC_SUBST(OSSIE_AFFINITY, "yes")
   AC_MSG_RESULT([yes])
else
  AC_MSG_RESULT([no])
fi

if test "x$enable_affinity" != "xno"; then
   AC_CHECK_LIB( numa, numa_available )
fi


AC_CONFIG_FILES([Makefile])
AC_OUTPUT


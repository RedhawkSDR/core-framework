#
# This file is protected by Copyright. Please refer to the COPYRIGHT file
# distributed with this source distribution.
#
# This file is part of REDHAWK core.
#
# REDHAWK core is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
#
# REDHAWK core is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see http://www.gnu.org/licenses/.
#

AC_INIT(frontendInterfaces, 2.3.3)

AM_INIT_AUTOMAKE(nostdinc)
AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_LIBTOOL

AM_PATH_PYTHON([2.3])

OSSIE_CHECK_OSSIE
OSSIE_OSSIEHOME_AS_PREFIX
OSSIE_PYTHON_INSTALL_SCHEME

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AC_CORBA_ORB
AC_CHECK_PROG([IDL], [omniidl], [omniidl], [no])
if test "$IDL" = no; then
  AC_MSG_ERROR([cannot find omniidl program])
fi
AC_LANG_PUSH([C++])
PKG_CHECK_MODULES([OMNIORB], [omniORB4 >= 4.1.0])
PKG_CHECK_MODULES([OSSIE], [ossie >= 2.0.0])
RH_PKG_IDLDIR([OSSIE], [ossie])

# If you depend on other IDL modules, such as CF or BULKIO add them here
PKG_CHECK_MODULES([BULKIO], [bulkioInterfaces >= 2.0])
AC_CHECK_PYMODULE(bulkio.bulkioInterfaces, [], [AC_MSG_ERROR([the python bulkio.bulkioInterfaces module is required])])
RH_PKG_IDLDIR([BULKIO], [bulkioInterfaces])

AC_SUBST([FRONTEND_SO_VERSION],[0:0:0])
AC_SUBST([FRONTEND_API_VERSION],[2.3])

AX_BOOST_BASE([1.41])
AX_BOOST_THREAD
OSSIE_ENABLE_LOG4CXX
CHECK_VECTOR_IMPL

AC_SUBST(FRONTEND_INF_INCLUDES, "-I../src/cpp -I../src/cpp/redhawk")
AC_SUBST(FRONTEND_INF_CFLAGS, )
AC_SUBST(FRONTEND_INF_LIBS, )

# Optionally include java support
AC_ARG_ENABLE([java], AS_HELP_STRING([--disable-java], [Disable framework java support]))

HAVE_JAVASUPPORT=no
if test "x$enable_java" != "xno"; then
  # Ensure JAVA_HOME is set
  RH_JAVA_HOME
  RH_PROG_JAVAC([1.6])
  RH_PROG_JAR
  RH_PROG_IDLJ

  # Set up build rules for jarfiles and IDL-to-Java
  RH_JARFILE_RULES
  RH_IDLJ_RULES

  # Find required libraries
  RH_PKG_CLASSPATH([OSSIE], [ossie])
  RH_PKG_CLASSPATH([BULKIO], [bulkio])

  # Set Java installation directory
  AC_SUBST([javadir], '${prefix}/lib')

  HAVE_JAVASUPPORT=yes
fi
AM_CONDITIONAL(HAVE_JAVASUPPORT, test $HAVE_JAVASUPPORT = yes)

AC_CONFIG_FILES([Makefile frontendInterfaces.pc])
if test "$enable_base_classes" != "no"; then
  if test "$HAVE_JAVASUPPORT = yes"; then
    AC_CONFIG_FILES([libsrc/java/META-INF/MANIFEST.MF])
  fi
  AC_CONFIG_FILES([libsrc/Makefile libsrc/frontend.pc])
fi
AC_OUTPUT

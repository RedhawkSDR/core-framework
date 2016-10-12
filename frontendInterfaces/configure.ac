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

AC_INIT(frontendInterfaces, 2.2.1)

AM_INIT_AUTOMAKE(nostdinc)
AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_LIBTOOL

AM_PATH_PYTHON([2.3])

OSSIE_CHECK_OSSIE
OSSIE_OSSIEHOME_AS_PREFIX
OSSIE_PYTHON_INSTALL_SCHEME

AC_CORBA_ORB
AC_CHECK_PROG([IDL], [omniidl], [omniidl], [no])
if test "$IDL" = no; then
  AC_MSG_ERROR([cannot find omniidl program])
fi
AC_LANG_PUSH([C++])
PKG_CHECK_MODULES([OMNIORB], [omniORB4 >= 4.1.0])
RH_PKG_IDLDIR([OMNIORB], [omniORB4])
PKG_CHECK_MODULES([OSSIE], [ossie >= 1.10.0])
RH_PKG_IDLDIR([OSSIE], [ossie])
PKG_CHECK_MODULES([OMNICOS], [omniCOS4 >= 4.0.0])
RH_PKG_IDLDIR([OMNICOS], [omniCOS4])

# If you depend on other IDL modules, such as CF or BULKIO add them here
PKG_CHECK_MODULES([BULKIO], [bulkioInterfaces >= 1.10])
AC_CHECK_PYMODULE(bulkio.bulkioInterfaces, [], [AC_MSG_ERROR([the python bulkio.bulkioInterfaces module is required])])
RH_PKG_IDLDIR([BULKIO], [bulkioInterfaces])

AC_SUBST([FRONTEND_SO_VERSION],[0:0:0])
AC_SUBST([FRONTEND_API_VERSION],[2.2.0])

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
  AC_ARG_VAR([JAVA_HOME], [Java Development Kit (JDK) location])
  test -r /usr/share/java-utils/java-functions && \
    . /usr/share/java-utils/java-functions && \
    set_jvm
  AC_MSG_CHECKING([for a valid JAVA_HOME])
  if test -n "$JAVA_HOME" -a -d "$JAVA_HOME"; then
    AC_MSG_RESULT([$JAVA_HOME])
    java_test_paths=$JAVA_HOME/jre/sh$PATH_SEPARATOR$JAVA_HOME/bin$PATH_SEPARATOR$PATH
  else
    AC_MSG_RESULT([no - this may impact Java tool detection])
    java_test_paths=$PATH
  fi

  # Locate the tools we need
  AC_PATH_PROG([JAVAC], [javac], [no], [$java_test_paths])
  AC_PATH_PROG([JAR], [jar], [no], [$java_test_paths])
  AC_PATH_PROG([IDLJ], [idlj], [no], [$java_test_paths])
  AS_IF([test "$JAVAC" = no -o "$JAR" = no -o "$IDLJ" = no],
        [AC_MSG_ERROR([Cannot find a required Java tool])])

  HAVE_JAVASUPPORT=yes
fi
AM_CONDITIONAL(HAVE_JAVASUPPORT, test $HAVE_JAVASUPPORT = yes)

AC_CONFIG_FILES(Makefile)
if test "$enable_base_classes" != "no"; then
  if test "$HAVE_JAVASUPPORT = yes"; then
    AC_CONFIG_FILES([libsrc/java/META-INF/MANIFEST.MF libsrc/java/META-INF/MANIFEST.MF.src])
  fi
  AC_CONFIG_FILES([libsrc/Makefile libsrc/frontend.pc])
fi
AC_OUTPUT

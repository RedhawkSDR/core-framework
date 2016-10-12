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

AC_INIT(burstio, 2.0.2)
AC_CONFIG_SRCDIR([src/cpp/Makefile.am])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([foreign])
AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_LIBTOOL

AM_PATH_PYTHON([2.4])

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

PKG_CHECK_MODULES(OSSIE, ossie >= 2.0.2,,exit)
RH_PKG_IDLDIR([OSSIE], [ossie])

PKG_CHECK_MODULES([BULKIO], [bulkioInterfaces >= 2.0])
RH_PKG_IDLDIR([BULKIO], [bulkioInterfaces])

AX_BOOST_BASE([1.41])
AX_BOOST_THREAD
AX_BOOST_SYSTEM

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

# Optionally include java support
AC_ARG_ENABLE([java], AS_HELP_STRING([--disable-java], [Disable java support]))

HAVE_JAVASUPPORT=no
if test "x$enable_java" != "xno"; then
  # configure was run with java enabled
  java_source_version=1.6

  RH_JAVA_HOME
  RH_PROG_JAVAC([$java_source_version])
  RH_PROG_JAR
  RH_JARFILE_RULES
  RH_PROG_IDLJ
  RH_IDLJ_RULES
  RH_JAVA_JNI_H

  # Set up CLASSPATH for REDHAWK, CF and BULKIO
  RH_PKG_CLASSPATH([OSSIE], [ossie])
  RH_PKG_CLASSPATH([BULKIO], [bulkioInterfaces])

  if test -n "$HAVE_JNI_H"; then
    # The omnijni package must be available to generate JNI stubs and skeletons.
    AC_CHECK_PYMODULE([ossie.omnijni], [HAVE_OMNIJNI="yes"])
  fi

  if test -n "$HAVE_OMNIJNI"; then
    # Configure IDLJNI and IDLJNICXX commands
    _RH_PROG_IDLJNI
    _RH_PROG_IDLJNICXX

    # Verify that we can include the CF and BULKIO JNI headers.
    AC_LANG_PUSH([C++])
    saved_CPPFLAGS="$CPPFLAGS"
    CPPFLAGS="$JNI_CPPFLAGS $OSSIE_CFLAGS $BULKIO_CFLAGS"
    AC_CHECK_HEADER([CF/jni_cf.h], [HAVE_JNI_CF_H="yes"])
    AC_CHECK_HEADER([BULKIO/jni_bulkioDataTypes.h], [HAVE_JNI_BULKIO_H="yes"])
    CPPFLAGS="$saved_CPPFLAGS"
    AC_LANG_POP([C++])
  fi

  if test -n "$HAVE_JNI_CF_H"; then
    # Check for the CF JNI library.
    AC_MSG_CHECKING([for ossiecfjni dynamic library])
    ossie_libdir=`pkg-config --variable=libdir ossie`
    if test -f "${ossie_libdir}/libossiecfjni.so"; then
      AC_MSG_RESULT([yes])
      CFJNI_LIBS="-L$ossie_libdir -lossiecfjni"
    else
      AC_MSG_RESULT([no])
    fi
  fi

  if test -n "$CFJNI_LIBS"; then
    # Check for the BULKIO JNI library
    AC_MSG_CHECKING([for bulkiojni dynamic library])
    bulkio_libdir=`pkg-config --variable=libdir bulkioInterfaces`
    if test -f "${bulkio_libdir}/libbulkiojni.so"; then
      AC_MSG_RESULT([yes])
      BULKIOJNI_LIBS="-L$bulkio_libdir -lbulkiojni"
    else
      AC_MSG_RESULT([no])
    fi
  fi

  if test -n "$BULKIOJNI_LIBS"; then
    HAVE_JAVASUPPORT="yes"
    AC_SUBST([AM_JAVACFLAGS], "-source $java_source_version")
    AC_SUBST([JNI_LIBS], "$CFJNI_LIBS $BULKIOJNI_LIBS")
    AC_SUBST([javadir], '${prefix}/lib')
  else
    HAVE_JAVASUPPORT="no"
  fi
fi
AC_MSG_CHECKING([for Java support])
AC_MSG_RESULT($HAVE_JAVASUPPORT)
AM_CONDITIONAL(HAVE_JAVASUPPORT, test $HAVE_JAVASUPPORT = yes)
# End optional java support

AC_SUBST(idldir, '${prefix}/share/idl')

AC_CONFIG_FILES([Makefile \
                 burstio.pc \
                 burstioInterfaces.pc \
                 src/cpp/Makefile \
                 src/java/Makefile \
                 src/python/Makefile \
                 src/python/setup.py \
                 src/idl/Makefile])

AC_OUTPUT

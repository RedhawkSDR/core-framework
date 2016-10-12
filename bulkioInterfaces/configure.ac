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

AC_INIT(bulkioInterfaces, 1.8.8)
AC_SUBST([LIBBULKIOINTERFACES_VERSION_INFO], [3:0:1])

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
PKG_CHECK_MODULES([OSSIE], [ossie >= 1.7.0])

# Optionally include java support
AC_ARG_ENABLE([java], AS_HELP_STRING([--disable-java], [Disable framework java support]))

HAVE_JAVASUPPORT=no
if test "x$enable_java" != "xno"; then
  # configure was run with java enabled

  # Ensure JAVA_HOME is set
  AC_ARG_VAR([JAVA_HOME], [Java Development Kit (JDK) location])
  test -r /usr/share/java-utils/java-functions && \
    . /usr/share/java-utils/java-functions && \
    set_jvm
  AC_MSG_CHECKING([for a valid JAVA_HOME])
  if test -n "$JAVA_HOME" -a -d "$JAVA_HOME"; then
    AC_MSG_RESULT([$JAVA_HOME])
  else
    AC_MSG_RESULT([no])
    AC_MSG_WARN([try setting the JAVA_HOME variable to the base of a valid Java install])
    AC_SUBST([JAVA_HOME], [no])
  fi

  # Locate tools we need based on JAVA_HOME
  java_test_paths=$JAVA_HOME/jre/sh$PATH_SEPARATOR$JAVA_HOME/bin
  AC_PATH_PROG([JAVAC], [javac], [no], [$java_test_paths])
  AC_PATH_PROG([JAR], [jar], [no], [$java_test_paths])
  AC_PATH_PROG([IDLJ], [idlj], [no], [$java_test_paths])

  if test "$JAVA_HOME" != "no" -a \
          "$JAVAC" != "no" -a \
          "$JAR" != "no" -a \
          "$IDLJ" != "no"; then
    # Verify that we can include the JNI header; the platform-specific location is
    # assumed to be Linux.
    saved_CPPFLAGS="$CPPFLAGS"
    JNI_CPPFLAGS="-I$JAVA_HOME/include -I$JAVA_HOME/include/linux"
    CPPFLAGS="$JNI_CPPFLAGS"
    AC_CHECK_HEADER([jni.h],
    [
      HAVE_JNI_H="yes"
      AC_SUBST([JNI_CPPFLAGS])
    ])
    CPPFLAGS="$saved_CPPFLAGS"
  fi

  if test -n "$HAVE_JNI_H"; then
    # The omnijni package must be available to generate JNI stubs and skeletons.
    AC_CHECK_PYMODULE([ossie.omnijni], [HAVE_OMNIJNI="yes"])
  fi

  if test -n "$HAVE_OMNIJNI"; then
    # Verify that we can include the CF JNI headers.
    saved_CPPFLAGS="$CPPFLAGS"
    CPPFLAGS="$JNI_CPPFLAGS $OSSIE_CFLAGS"
    AC_CHECK_HEADER([CF/jni_cf.h], [HAVE_JNI_CF_H="yes"])
    CPPFLAGS="$saved_CPPFLAGS"
  fi

  if test -n "$HAVE_JNI_CF_H"; then
    # Finally, check for the CF JNI module.
    AC_MSG_CHECKING([for ossiecfjni dynamic library])
    ossie_libdir=`pkg-config --variable=libdir ossie`
    if test -f "${ossie_libdir}/libossiecfjni.so"; then
      AC_MSG_RESULT([yes])
      JNI_LIBS="-L$ossie_libdir -lossiecfjni"
      AC_SUBST([JNI_LIBS])
      HAVE_JAVASUPPORT="yes"
    else
      AC_MSG_RESULT([no])
    fi
  fi
fi
AC_MSG_CHECKING([for framework java support])
AC_MSG_RESULT($HAVE_JAVASUPPORT)
AM_CONDITIONAL(HAVE_JAVASUPPORT, test $HAVE_JAVASUPPORT = yes)
# End optional java support

AC_CONFIG_FILES(Makefile jni/Makefile)
AC_OUTPUT

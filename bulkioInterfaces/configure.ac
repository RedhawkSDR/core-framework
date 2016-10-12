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

AC_INIT(bulkioInterfaces, 1.10.2)
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
PKG_CHECK_MODULES([OMNIORB], [omniORB4 >= 4.1.0])
PKG_CHECK_MODULES([OSSIE], [ossie >= 1.9.0])

RH_PKG_IDLDIR([OSSIE], [ossie])

# Optionally include BULKIO base class libraries
AC_ARG_ENABLE([base-classes], AS_HELP_STRING([--disable-base-classes], [Disable BULKIO base class libraries]))
AM_CONDITIONAL([BUILD_BASE_CLASSES], [test "$enable_base_classes" != "no"])

if test "$enable_base_classes" != "no"; then
  AC_SUBST([BULKIO_SO_VERSION], [0:0:0])
  AC_SUBST([BULKIO_API_VERSION], [1.10])

  AX_BOOST_BASE([1.41])
  AX_BOOST_THREAD
  AX_BOOST_SYSTEM
  OSSIE_ENABLE_LOG4CXX
  CHECK_VECTOR_IMPL

  AC_SUBST(BULKIO_INF_INCLUDES, "-I../src/cpp -I../src/cpp/ossie")
  AC_SUBST(BULKIO_INF_CFLAGS, )
  AC_SUBST(BULKIO_INF_LIBS, )
fi

# Optionally include java support
AC_ARG_ENABLE([java], AS_HELP_STRING([--disable-java], [Disable framework java support]))

HAVE_JAVASUPPORT=no
if test "x$enable_java" != "xno"; then
  # configure was run with java enabled

  java_source_version=1.6

  RH_JAVA_HOME
  RH_PROG_JAVAC([$java_source_version])
  RH_PROG_JAR
  RH_PROG_IDLJ

  # Set up build rules for jarfiles and IDL-to-Java
  RH_JARFILE_RULES
  RH_IDLJ_RULES

  RH_JAVA_JNI_H

  RH_PKG_CLASSPATH([OSSIE], [ossie])

  if test -n "$HAVE_JNI_H"; then
    # The omnijni package must be available to generate JNI stubs and skeletons.
    AC_CHECK_PYMODULE([ossie.omnijni], [HAVE_OMNIJNI="yes"])
  fi

  if test -n "$HAVE_OMNIJNI"; then
    _RH_PROG_IDLJNI
    _RH_PROG_IDLJNICXX

    # Verify that we can include the CF JNI headers.
    AC_LANG_PUSH([C++])
    saved_CPPFLAGS="$CPPFLAGS"
    CPPFLAGS="$JNI_CPPFLAGS $OSSIE_CFLAGS"
    AC_CHECK_HEADER([CF/jni_cf.h], [HAVE_JNI_CF_H="yes"])
    CPPFLAGS="$saved_CPPFLAGS"
    AC_LANG_POP([C++])
  fi

  if test -n "$HAVE_JNI_CF_H"; then
    # Finally, check for the CF JNI module.
    AC_MSG_CHECKING([for ossiecfjni dynamic library])
    ossie_libdir=`pkg-config --variable=libdir ossie`
    if test -f "${ossie_libdir}/libossiecfjni.so"; then
      AC_MSG_RESULT([yes])

      # Set Java installation directory
      AC_SUBST([javadir], '${prefix}/lib')

      JNI_LIBS="-L$ossie_libdir -lossiecfjni"
      AC_SUBST([JNI_LIBS])

      # Set the -source flag to the minimum Java version
      AC_SUBST(AM_JAVACFLAGS, "-source $java_source_version")
      AC_SUBST(JAVA_SOURCE_VERSION, "$java_source_version")

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

AC_CONFIG_FILES([bulkioInterfaces.pc setup.py Makefile jni/Makefile])
if test "$enable_base_classes" != "no"; then
  if test "$HAVE_JAVASUPPORT = yes"; then
    AC_CONFIG_FILES([libsrc/java/META-INF/MANIFEST.MF libsrc/java/META-INF/MANIFEST.MF.src])
  fi
  AC_CONFIG_FILES([libsrc/Makefile libsrc/bulkio.pc])
fi

AC_OUTPUT

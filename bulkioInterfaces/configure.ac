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

AC_INIT(bulkioInterfaces, 2.2.6)
AC_CONFIG_MACRO_DIR([m4])
AC_SUBST([LIBBULKIOINTERFACES_VERSION_INFO], [3:0:1])

AM_INIT_AUTOMAKE([foreign nostdinc subdir-objects])
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
RH_PKG_IDLDIR([OMNIORB], [omniORB4])

PKG_CHECK_MODULES([OSSIE], [ossie >= 2.2.6])
RH_PKG_IDLDIR([OSSIE], [ossie])
PKG_CHECK_MODULES([OMNICOS], [omniCOS4 >= 4.0.0])
RH_PKG_IDLDIR([OMNICOS], [omniCOS4])

AC_SUBST([BULKIO_SO_VERSION], [0:0:0])
AC_SUBST([BULKIO_API_VERSION], [2.2])

AX_BOOST_BASE([1.41])
AX_BOOST_THREAD
AX_BOOST_SYSTEM
OSSIE_ENABLE_LOG4CXX
CHECK_VECTOR_IMPL

AC_SUBST(BULKIOINTERFACES_CFLAGS, "-I \$(top_srcdir)/src/cpp -I \$(top_srcdir)/src/cpp/ossie")
AC_SUBST(BULKIOINTERFACES_LIBS, "-L\$(top_srcdir) -lbulkioInterfaces")

# Optionally include java support
AC_ARG_ENABLE([java], AS_HELP_STRING([--disable-java], [Disable framework java support]))

HAVE_JAVASUPPORT=no
if test "x$enable_java" != "xno"; then
  # configure was run with java enabled

  java_source_version=1.8

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

# C++ unit testing support. May want to conditionally enable/disable this.
AM_PATH_CPPUNIT(1.12.1)
AS_IF([test "x$HAVE_JAVASUPPORT" == "xyes"], [
  dnl Use RPM location hard-coded for now
  AC_SUBST([JUNIT_CLASSPATH], "/usr/share/java/junit4.jar")
])

# For C++ test components, provide BULKIO_CFLAGS and BULKIO_LIBS so they build
# without modifying their Makefile.am to change the paths. In order to pick up
# the uninstalled headers, we need to provide the path to the BULKIO headers,
# the top-level include directory ("bulkio/bulkio.h") and the bulkio
# directory inside of include ("bulkio.h").
bulkio_includedir="\$(top_srcdir)/libsrc/cpp/include"
AC_SUBST(BULKIO_CFLAGS, "-I ${bulkio_includedir} -I ${bulkio_includedir}/bulkio ${BULKIOINTERFACES_CFLAGS}")
AC_SUBST(BULKIO_LIBS, "-L\$(top_srcdir)/libsrc -lbulkio-${BULKIO_API_VERSION} ${BULKIOINTERFACES_LIBS}")

AC_CONFIG_FILES([bulkioInterfaces.pc setup.py Makefile jni/Makefile])
if test "$HAVE_JAVASUPPORT = yes"; then
  AC_CONFIG_FILES([libsrc/java/META-INF/MANIFEST.MF])
fi
AC_SUBST(PROJECTDEPS_CFLAGS, "\$(OSSIE_CFLAGS)")
AC_SUBST(PROJECTDEPS_LIBS, "\$(OSSIE_LIBS)")
AC_SUBST(INTERFACEDEPS_CFLAGS, "\$(BULKIO_CFLAGS)")
AC_SUBST(INTERFACEDEPS_LIBS, "\$(BULKIO_LIBS)")
AC_SUBST(REDHAWK_CLASSPATH, "\$(OSSIE_CLASSPATH)")
AC_SUBST(BULKIO_CLASSPATH, "\$(top_srcdir)/libsrc/bulkio.jar:\$(top_srcdir)/BULKIOInterfaces.jar")
AC_CONFIG_FILES([libsrc/Makefile \
                 libsrc/bulkio.pc \
                 libsrc/testing/Makefile \
                 libsrc/testing/tests/cpp/Makefile \
                 libsrc/testing/tests/transport_api/Makefile \
                 libsrc/testing/tests/java/Makefile \
                 libsrc/testing/components/CPP_Ports/cpp/Makefile \
                 libsrc/testing/components/Java_Ports/java/Makefile \
                 libsrc/testing/components/Oversized_framedata/cpp/Makefile \
                 libsrc/testing/components/src/cpp/Makefile \
                 libsrc/testing/components/snk_slow/cpp/Makefile \
                 libsrc/testing/components/Oversized_framedata/java/Makefile \
                 libsrc/testing/components/TestLargePush/cpp/Makefile \
                 libsrc/testing/components/TestLargePush/java/Makefile \
                 libsrc/testing/components/multiout_attachable/cpp/Makefile \
                 libsrc/testing/components/multiout_attachable/java/Makefile \
                 libsrc/testing/components/sri_changed_cpp/cpp/Makefile \
                 libsrc/testing/devices/dev_snk/java/Makefile \
                 libsrc/testing/devices/dev_src/java/Makefile])

AC_OUTPUT

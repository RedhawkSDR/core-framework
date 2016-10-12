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

dnl Update this version number immedately after a release
AC_INIT([ossie],[2.0.3])
#AM_INIT_AUTOMAKE(nostdinc)
# allows filenames over 99 characters long during dist
AM_INIT_AUTOMAKE([1.9 tar-pax subdir-objects])
dnl Update these verion numbers immedately before a release
dnl
dnl The version number is in the format "current:revision:age"
dnl
dnl Follow these steps (in order) to correctly update the
dnl version info:
dnl
dnl 1. If any source code has changed at all, increment revision
dnl 2. If any interfaces have been added, modified, or removed
dnl    then increment current and set revision to 0
dnl 3. If any interfaces have been addded then increment age
dnl 4. If any interfaces have been removed or changed, then set
dnl    age to 0
AC_SUBST([LIBOSSIECF_VERSION_INFO], [4:0:0])
AC_SUBST([LIBOSSIEPARSER_VERSION_INFO], [3:0:0])
AC_SUBST([LIBOMNIJNI_VERSION_INFO], [1:0:1])
AC_SUBST([LIBOSSIECFJNI_VERSION_INFO], [1:0:0])
AC_SUBST([LIBREDHAWK_LOGGING_VERSION_INFO], [1:0:0])


dnl AC_CONFIG_MACRO_DIR([m4])
dnl LT_INIT

AC_PREFIX_DEFAULT([/usr/local/redhawk/core])
OSSIE_OSSIEHOME_AS_PREFIX
OSSIE_SDRROOT_IGNORE_PREFIX

AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AC_PROG_LIBTOOL
PKG_PROG_PKG_CONFIG
AC_HEADER_SYS_WAIT

AC_CORBA_ORB

AC_LANG_PUSH([C++])

AX_BOOST_BASE([1.35])
AX_BOOST_SYSTEM
AX_BOOST_FILESYSTEM
AX_BOOST_SERIALIZATION
AX_BOOST_REGEX
AX_BOOST_THREAD
AX_BOOST_DATE_TIME

AX_LIB_EXPAT([1.95.8])
if test x"$HAVE_EXPAT" = xno ; then
        AC_MSG_ERROR([The Expat XML parser library and header files are required])
fi

PKG_CHECK_MODULES([OMNIORB], [omniORB4 >= 4.0.0])
PKG_CHECK_MODULES([OMNITHREAD], [omnithread3 >= 4.0.0])
PKG_CHECK_MODULES([OMNIDYNAMIC], [omniDynamic4 >= 4.0.0])

dnl Use PKG_CHECK_EXISTS because it won't throw error messages like PKG_CHECK_MODULES will
AC_MSG_CHECKING([for COS library])
PKG_CHECK_EXISTS([omniCOS4 >= 4.0.0], [libomnicos_exist=yes], [libomnicos_exist=no])
dnl Use PKG_CHECK_MODULES to declare the CFLAGS and LIBS varible
AC_MSG_RESULT([yes])
PKG_CHECK_MODULES([OMNICOS], [omniCOS4 >= 4.0.0])
OMNICOS_IDLDIR=`pkg-config --variable=idldir omniCOS4`
AC_SUBST(OMNICOS_IDLDIR)

AC_DEFINE(ENABLE_EVENTS, 1, [Define if events are enabled])
AC_SUBST(ENABLE_EVENTS_FLAGS, [-DENABLE_EVENTS=1])
AM_CONDITIONAL(ENABLE_EVENTS, [true])

AC_CHECK_PROG([IDL], [omniidl], [omniidl], [no])
if test "$IDL" = no; then
  AC_MSG_ERROR([cannot find omniidl program])
fi

AC_MSG_CHECKING([for default IDL path])
idldir=`pkg-config --variable=idldir omniORB4`
AC_MSG_RESULT([$idldir])
AC_SUBST(IDLDIR, [$idldir])

dnl Executable is named xsdcxx in Fedora epel repo, REDHAWK dep is named xsd
AC_CHECK_PROGS([XSD], [xsdcxx xsd], [no])
if test x"$XSD" = xno ; then
	AC_MSG_ERROR([Required program 'xsdcxx' is missing.])
fi
# Get the XSD version
AC_MSG_CHECKING([xsd version])
xsd_version=`${XSD} version 2>&1 | head -n 1 | awk '{print $8}'`
oldest_version=`echo -e "${xsd_version}\n3.3.0" | sort -t '.' -k 1,1 -k 2,2 -k 3,3 -k 4,4 -g | head -n 1`
if test "x$oldest_version" != "x3.3.0"; then
    AC_MSG_FAILURE([XSD version 3.3.0 required])
else
    AC_MSG_RESULT([$xsd_version])
fi
AC_CHECK_HEADER([xsd/cxx/config.hxx], [], [AC_MSG_ERROR([missing XSD headers])])
AC_CHECK_PROG([XMLCATALOG], [xmlcatalog], [xmlcatalog], [no])
AM_CONDITIONAL(HAVE_XMLCATALOG, test $XMLCATALOG != "no")

# add numa support to affinity module
AC_ARG_ENABLE([affinity], AS_HELP_STRING([--enable-affinity], [Enable numa affinity processing]))
   AC_MSG_CHECKING([Adding support for numa affinity processing])
if test "x$enable_affinity" = "xyes"; then
   dnl  determine if numa library support is available
   AC_SUBST(OSSIE_AFFINITY, "yes")
   AC_MSG_RESULT([yes])
else
  AC_MSG_RESULT([no])
fi

if test "x$enable_affinity" = "xyes"; then
   AC_CHECK_LIB( numa, numa_available )
fi


# Find a Posix-compliant sed
# We attempt to use AC_PROG_SED, but it's only available in autoconf >= 2.59b
# Otherwise, settle for finding it by executable name
ifdef([AC_PROG_SED], [AC_PROG_SED], [AC_CHECK_PROGS(SED, [gsed sed])])

OSSIE_PYTHON_INSTALL_SCHEME
OSSIE_ENABLE_LOG4CXX
OSSIE_ENABLE_TRACE
OSSIE_ENABLE_PERSISTENCE

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AC_ARG_ENABLE([xml_validation], AS_HELP_STRING([--disable-xml-validation], [Disable xml validation in the parsers]))
AC_MSG_CHECKING([if xml parsers should perform validation])
if test "x$enable_xml_validation" != "xno"; then
  AC_SUBST(OSSIE_XSDFLAGS, "--generate-validation")
  AC_MSG_RESULT([yes])
else
  AC_SUBST(OSSIE_XSDFLAGS, "--suppress-validation")
  AC_MSG_RESULT([no])
fi

# Allow user to explictly prevent JAVA compilation
AC_ARG_ENABLE([java], AS_HELP_STRING([--disable-java], [Disable framework java support]))

HAVE_JAVASUPPORT=no
if test "x$enable_java" != "xno"; then
  # configure was run with java enabled

  # Ensure JAVA_HOME is set
  RH_JAVA_HOME

  # Minimum Java version
  java_source_version=1.6

  # Locate tools we need based on JAVA_HOME
  RH_PROG_JAVAC([$java_source_version])
  RH_PROG_JAR
  RH_PROG_IDLJ

  # Enable Java Automake-like build rules
  RH_JARFILE_RULES

  # Enable IDL-to-Java build rules
  RH_IDLJ_RULES

  # Check vendor
  RH_JAVA_VENDOR

  # Locate headers we need
  RH_JAVA_JNI_H

  if test "$JAVA_HOME" != "no" -a \
          "$JAVAC" != "no" -a \
          "$JAR" != "no" -a \
          "$IDLJ" != "no" -a \
          "$JAVA_VENDOR" == "yes" -a \
          "$HAVE_JNI_H" == "yes"; then
    HAVE_JAVASUPPORT="yes"
    AC_SUBST(javadir, '${prefix}/lib')

    # Set the -source flag to the minimum Java version
    AC_SUBST(AM_JAVACFLAGS, "-source $java_source_version")
  else
    AC_MSG_WARN([Java support was enabled, but Java requirements were not met])
  fi
fi

AC_SUBST([HAVE_JAVASUPPORT], $HAVE_JAVASUPPORT)
AM_CONDITIONAL(HAVE_JAVASUPPORT, test $HAVE_JAVASUPPORT = yes)

AM_PATH_PYTHON([2.4])

AC_CHECK_PYMODULE(logging ,[], [AC_MSG_ERROR([the python logging module is required])])
AC_CHECK_PYMODULE(subprocess, [], [AC_MSG_ERROR([the python subprocess module is required])])
AC_CHECK_PYMODULE(omniORB, [], [AC_MSG_ERROR([the python omniORB module is required])])
if test -d /usr/lib/omniidl; then # for debian-based systems
  export PYTHONPATH=${PYTHONPATH}:/usr/lib/omniidl
fi
AC_CHECK_PYMODULE(omniidl, [], [AC_MSG_ERROR([the python omniidl module is required])])

AC_LANG_POP
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS(time.h)
AC_HEADER_DIRENT
AX_CXX_GCC_ABI_DEMANGLE

if test "x$cross_compiling" == "xyes"; then
# Cross our fingers and hope
  ac_cv_func_fork_works="yes"
else
  AC_FUNC_FORK
  if test "$ac_cv_func_fork_works" != yes; then
     AC_MSG_ERROR([working fork test failed. Try running /sbin/ldconfig.])
  fi
fi

# Use libuuid from e2fsprogs to generate UUIDs.
AC_CHECK_HEADER([uuid/uuid.h], [], [AC_MSG_ERROR([uuid/uuid.h is required])])
AC_CHECK_LIB([uuid], [uuid_generate], [], [AC_MSG_ERROR([libuuid is required])])

# Set flags for building against the main C++ library.
AC_SUBST(OSSIE_CFLAGS, '-I$(top_srcdir)/base/include')
AC_SUBST(OSSIE_IDLDIR, '$(top_srcdir)/idl')
AC_SUBST(OSSIEIDL_LIBS, '$(top_builddir)/base/framework/idl/libossieidl.la')

if test "$HAVE_JAVASUPPORT" = yes; then
  AC_SUBST(OSSIE_CLASSPATH, '$(top_srcdir)/base/framework/java/CFInterfaces.jar:$(top_srcdir)/base/framework/java/log4j-1.2.15.jar:$(top_srcdir)/base/framework/java/ossie/ossie.jar')

  # Set up omnijni-required variables
  # Enable other parts of the framework to locate omnijni
  AC_SUBST(omnijni_srcdir, '$(top_srcdir)/omnijni')
  AC_SUBST(omnijni_builddir, '$(top_builddir)/omnijni')

  # Install omnijni's Python files per the install scheme (it does not use a setup.py).
  if test "$PYTHON_INSTALL_SCHEME" = "home"; then
    pythondir='${prefix}/lib/python'
    pkgpythondir='${prefix}/lib/python/ossie'
  fi

  # Set flags and variables for building with omnijni
  AC_SUBST(OMNIJNI_CPPFLAGS, '$(JNI_CPPFLAGS) -I$(omnijni_srcdir)/include -I$(omnijni_builddir)/src/cpp/CORBA')
  AC_SUBST(OMNIJNI_LIBS, '$(omnijni_builddir)/src/cpp/libomnijni.la')
  AC_SUBST(OMNIJNI_PYTHONDIR, '$(omnijni_srcdir)/src/python')

  # Point IDLJNI to the source tree location, so the same makefile fragments
  # can support installed and internal builds.
  _RH_PROG_IDLJNI
  IDLJNI='$(IDL) -p $(OMNIJNI_PYTHONDIR) -b omnijni.idljava'

  # Likewise, point IDLJNICXX to the source tree location, though it is not
  # referenced outside of the JNI Makefile.am files
  _RH_PROG_IDLJNICXX
  IDLJNICXX='$(IDL) -p $(OMNIJNI_PYTHONDIR) -b omnijni.idljni'
fi

# Undefine the PACKAGE_ variables to avoid warnings when omniORB is installed outside of
# /usr.  OSSIE doesn't use these variables anyways...and even if it did
# they would get undefined in any file that included CORBA.h (because ossieconfig.h
# undefines them)
CPPFLAGS="${CPPFLAGS} -UPACKAGE_NAME -UPACKAGE_BUGREPORT -UPACKAGE_TARNAME -UPACKAGE_STRING -UPACKAGE_VERSION"


AC_CONFIG_FILES(Makefile \
                ossie.pc \
                acinclude/Makefile \
                etc/Makefile \
                omnijni/Makefile \
                omnijni/include/omnijni/Makefile \
                omnijni/src/cpp/Makefile \
                omnijni/src/cpp/CORBA/Makefile \
                omnijni/src/java/Makefile \
                omnijni/src/python/omnijni/Makefile \
                control/Makefile \
                control/parser/Makefile \
                control/framework/Makefile \
                control/sdr/Makefile \
                control/sdr/dommgr/Makefile \
                control/sdr/devmgr/Makefile \
                base/Makefile \
                base/framework/python/Makefile \
                base/framework/java/Makefile \
                base/framework/java/jni/Makefile \
                base/framework/java/jni/COS/Makefile \
                base/framework/java/ossie/Makefile \
                base/framework/idl/Makefile \
                base/framework/Makefile \
                base/include/ossie/Makefile \
                base/include/Makefile \
                base/parser/Makefile \
                base/plugin/Makefile \
                base/plugin/logcfg/Makefile \
                idl/Makefile \
                xml/Makefile \
                tools/Makefile \
                tools/LogEventAppender/Makefile \
                testing/Makefile \
                testing/_unitTestHelpers/buildconfig.py \
                testing/sdr/dev/devices/ExecutableDevice/Makefile \
                testing/sdr/dev/devices/BasicTestDevice_cpp/BasicTestDevice_cpp_impl1/Makefile \
                testing/sdr/dev/devices/BasicTestDevice_java/java/Makefile \
                testing/sdr/dev/devices/CppTestDevice/cpp/Makefile \
                testing/sdr/dev/devices/BasicDevWithExecParam_cpp/BasicDevWithExecParam_cpp_impl1/Makefile \
                testing/sdr/dev/devices/JavaTestDevice/java/Makefile \
                testing/sdr/dev/devices/SimpleDevice/SimpleDevice_cpp_impl1/Makefile \
                testing/sdr/dev/devices/props_test_device/props_test_device_cpp_impl1/Makefile \
                testing/sdr/dev/devices/issue_111_cpp/cpp/Makefile \
                testing/sdr/dev/devices/issue_111_java/java/Makefile \
                testing/sdr/dev/devices/PersonaDevice/cpp/Makefile \
                testing/sdr/dev/devices/ProgrammableDevice/cpp/Makefile \
                testing/sdr/dev/devices/cpp_dev/cpp/Makefile \
                testing/sdr/dev/devices/GPP/cpp/Makefile \
                testing/sdr/dev/devices/java_dev/java/Makefile \
                testing/sdr/dev/services/BasicService_java/java/Makefile \
                testing/sdr/dom/deps/cpp_dep1/cpp/Makefile \
                testing/sdr/dom/deps/cpp_dep1/cpp/cpp_dep1.pc \
                testing/sdr/dom/deps/cpp_dep2/cpp/Makefile \
                testing/sdr/dom/deps/cpp_dep2/cpp/cpp_dep2.pc \
                testing/sdr/dom/components/CppCallbacks/cpp/Makefile \
                testing/sdr/dom/components/prop_trigger_timing/cpp/Makefile \
                testing/sdr/dom/components/TestCppProps/Makefile \
                testing/sdr/dom/components/TestJavaProps/Makefile \
                testing/sdr/dom/components/TestLegacyJavaProps/Makefile \
                testing/sdr/dom/components/TestJavaPropsRange/java/Makefile \
                testing/sdr/dom/components/linkedLibraryTest/Makefile \
                testing/sdr/dom/components/TestCppsoftpkgDeps/Makefile \
                testing/sdr/dom/components/javaDep/javaDep/Makefile \
                testing/sdr/dom/components/java_comp/java/Makefile \
                testing/sdr/dom/components/HardLimit/HardLimit_java_impl1/Makefile \
                testing/sdr/dom/components/SimpleComponent/SimpleComponent_cpp_impl1/Makefile \
                testing/sdr/dom/components/BasicAC/basicac_java_impl1/Makefile \
                testing/sdr/dom/components/BasicAC/BasicAC_cpp_impl1/Makefile \
                testing/sdr/dom/components/MessageReceiverCpp/Makefile \
                testing/sdr/dom/components/MessageSenderCpp/Makefile \
                testing/sdr/dom/components/EventSend/EventSend_java_impl1/Makefile \
                testing/sdr/dom/components/EventReceive/EventReceive_java_impl1/Makefile \
                testing/sdr/dom/components/PropertyChangeEventsCpp/Makefile \
                testing/sdr/dom/components/PropertyChangeEventsJava/PropertyChangeEventsJava_java_impl1/Makefile \
                testing/sdr/dom/components/ticket2093/cpp/Makefile \
                testing/sdr/dom/components/TestComplexProps/cpp/Makefile \
                testing/sdr/dom/components/TestComplexProps/java/Makefile \
                testing/sdr/dom/components/TestCppPropsRange/cpp/Makefile \
                testing/sdr/dom/components/TestAllPropTypes/cpp/Makefile \
                testing/sdr/dom/components/TestAllPropTypes/java/Makefile \
                testing/sdr/dom/components/TestLoggingAPI/cpp/Makefile \
                testing/sdr/dom/components/cpp_comp/cpp/Makefile \
                testing/sdr/dom/components/cpp_with_deps/cpp/Makefile \
                testing/sdr/dom/components/TestLoggingAPI/java/Makefile \
                testing/sdr/dom/components/C1/cpp/Makefile \
                testing/sdr/dom/components/C2/cpp/Makefile \
                testing/sdr/dom/components/ECM_CPP/cpp/Makefile \
                testing/sdr/dom/components/ECM_JAVA/java/Makefile \
                testing/sdr/dom/components/PropertyChange_C1/cpp/Makefile \
                testing/sdr/dom/components/PropertyChange_J1/java/Makefile \
                testing/sdr/dom/components/Property_CPP/cpp/Makefile \
                testing/sdr/dom/components/commandline_prop/cpp/Makefile \
                testing/sdr/dom/components/nocommandline_prop/cpp/Makefile \
                testing/sdr/dom/components/Property_JAVA/java/Makefile \
                testing/sdr/dom/components/foo/bar/comp/cpp/Makefile \
                testing/sdr/dom/components/foo/bar/jv/java/Makefile \
                testing/sdr/dom/components/TestCppOptionalProps/cpp/Makefile \
                testing/sdr/dom/components/TestJavaOptionalProps/java/Makefile )
AC_OUTPUT


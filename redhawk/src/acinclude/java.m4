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

dnl _RH_SILENT_COMMAND
AC_DEFUN([_RH_SILENT_COMMAND],
[
  dnl Older versions of Automake that don't have AM_SILENT_RULES will complain
  dnl about AM_DEFAULT_VERBOSITY being undefined, so skip the entire process if
  dnl that's the case
  m4_ifdef([AM_SILENT_RULES], [
    dnl If AM_SILENT_RULES has been run, AM_DEFAULT_VERBOSITY will be set
    if test -n "$AM_DEFAULT_VERBOSITY"; then
      AC_SUBST([RH_V_][$1], '$(rh__v_'$1'_$(V))')
      AC_SUBST([rh__v_][$1][_], '$(rh__v_'$1'_$(AM_DEFAULT_VERBOSITY))')
      AC_SUBST([rh__v_][$1][_0], [$2])
    fi
  ])
])

dnl AC_JAVA_HOME
AC_DEFUN([RH_JAVA_HOME],
[
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
])

dnl RH_PROG_JAVA
AC_DEFUN([RH_PROG_JAVA],
[
  AC_REQUIRE([RH_JAVA_HOME])
  java_test_paths=$JAVA_HOME/jre/sh$PATH_SEPARATOR$JAVA_HOME/bin
  AC_PATH_PROG([JAVA], [java], [no], [$java_test_paths])
])

dnl RH_PROG_JAVAC(targetVersion)
dnl targetVersion is optional
AC_DEFUN([RH_PROG_JAVAC],
[
  AC_REQUIRE([RH_JAVA_HOME])
  AC_ARG_VAR([JAVACFLAGS], [Java compiler flags])
  java_test_paths=$JAVA_HOME/jre/sh$PATH_SEPARATOR$JAVA_HOME/bin
  AC_PATH_PROG([JAVAC], [javac], [no], [$java_test_paths])
  if test -n "$1" -a "$JAVAC" != "no"; then
    AC_MSG_CHECKING([javac version $1 compliance])
    cat << EOF > Test.java
      import java.util.Properties;
      public class Test {
	public static void main(String[[]] args) {
	}
      }
EOF

    if AC_TRY_COMMAND([$JAVAC -source $1 -target $1 Test.java]); then
      AC_MSG_RESULT([yes])
      _RH_SILENT_COMMAND([JAVAC], '@echo "  JAVAC " $[@];')
    else
      AC_MSG_RESULT([no])
      AC_SUBST([JAVAC], [no])
    fi 
    rm -f Test.java Test.class
  fi
])

dnl RH_PROG_JAR
AC_DEFUN([RH_PROG_JAR],
[
  AC_REQUIRE([RH_JAVA_HOME])
  java_test_paths=$JAVA_HOME/jre/sh$PATH_SEPARATOR$JAVA_HOME/bin
  AC_PATH_PROG([JAR], [jar], [no], [$java_test_paths])
  _RH_SILENT_COMMAND([JAR], '@echo "  JAR   " $[@];')
])

dnl RH_PROG_IDLJ
AC_DEFUN([RH_PROG_IDLJ],
[
  AC_REQUIRE([RH_JAVA_HOME])
  java_test_paths=$JAVA_HOME/jre/sh$PATH_SEPARATOR$JAVA_HOME/bin
  AC_PATH_PROG([IDLJ], [idlj], [no], [$java_test_paths])
  _RH_SILENT_COMMAND([IDLJ], '@echo "  IDLJ  " $<;')
])

dnl Check the value of a java property
dnl RH_GET_JAVA_PROPERTY(variable, propname)
AC_DEFUN([RH_GET_JAVA_PROPERTY],
[
  AC_REQUIRE([RH_PROG_JAVA])
  AC_REQUIRE([RH_PROG_JAVAC])
  AC_CACHE_CHECK([for java property $2], [$1],
  [
    cat << EOF > Test.java

import java.util.Properties;
public class Test {
  public static void main(String[[]] args) {
    Properties props = System.getProperties();
    System.out.println(props.getProperty("$2"));
  }
}

EOF
    $JAVAC Test.java &> /dev/null
    $1=`$JAVA -cp . Test`
    rm -f Test.java Test.class
  ])
  AC_DEFINE($1, cv_java_prop_$1)
])

dnl RH_JAVA_JNI_H
AC_DEFUN([RH_JAVA_JNI_H],
[
  AC_REQUIRE([RH_JAVA_HOME])

  # Verify that we can include the JNI header; the platform-specific location is
  # assumed to be Linux.
  saved_CPPFLAGS="$CPPFLAGS"
  JNI_CPPFLAGS="-I$JAVA_HOME/include -I$JAVA_HOME/include/linux"
  CPPFLAGS="$JNI_CPPFLAGS"
  AC_CHECK_HEADER([jni.h],
  [
    HAVE_JNI_H="yes"
    AC_SUBST([JNI_CPPFLAGS], [$JNI_CPPFLAGS])
  ])
  CPPFLAGS="$saved_CPPFLAGS"
])

dnl RH_JAVA_VENDOR
AC_DEFUN([RH_JAVA_VENDOR],
[
  RH_GET_JAVA_PROPERTY([_cv_JAVA_VENDOR], [java.vendor])
  AC_MSG_CHECKING([for an acceptable Java vendor])
  if ((test "$_cv_JAVA_VENDOR" != "Sun Microsystems Inc.") && (test "$_cv_JAVA_VENDOR" != "Oracle Corporation")); then
    AC_MSG_RESULT([no])
    JAVA_VENDOR="no"
  else
    AC_MSG_RESULT([yes])
    JAVA_VENDOR="yes"
  fi
])

dnl Automake targets. Referencing via variables rather lets Automake generate its
dnl normal output; otherwise, literal references to certain targets overrule the
dnl Automake versions entirely.
AC_DEFUN([_RH_AUTOMAKE_TARGETS],
[
  AC_SUBST([RH_TARGET_MAKEFILE], [Makefile])
  AC_SUBST([RH_TARGET_ALL_AM], [all-am])
  AC_SUBST([RH_TARGET_CLEAN_AM], [clean-am])
  AC_SUBST([RH_TARGET_DISTCLEAN_AM], [distclean-am])
  AC_SUBST([RH_TARGET_INSTALL_DATA_AM], [install-data-am])
  AC_SUBST([RH_TARGET_UNINSTALL_AM], [uninstall-am])
])

dnl For autoconf pre-2.60 versions, get MKDIR_P from automake
m4_ifdef([AC_PROG_MKDIR_P], [], [
  AC_DEFUN([AC_PROG_MKDIR_P], [
    AC_REQUIRE([AM_PROG_MKDIR_P])
    MKDIR_P='$(mkdir_p)'
    AC_SUBST([MKDIR_P])
  ])
])

dnl RH_JARFILE_RULES
dnl Sets up requirements for building Java jarfiles using the distributed
dnl makefile fragment
AC_DEFUN([RH_JARFILE_RULES],
[
  AC_REQUIRE([RH_PROG_JAVAC])
  AC_REQUIRE([RH_PROG_JAR])
  AC_REQUIRE([AC_PROG_MKDIR_P])dnl required for install
  AC_REQUIRE([_RH_AUTOMAKE_TARGETS])

  dnl Set a variable to point to the makefile fragment, which can then be used
  dnl in place of the actual path
  AC_SUBST([rh_jarfile_rules], 'include $(OSSIE_HOME)/share/aminclude/redhawk/jarfile.am')
])

dnl _RH_PROG_IDLJNI (for internal use only)
AC_DEFUN([_RH_PROG_IDLJNI],
[
  AC_SUBST([IDLJNI], '$(IDL) -p $(OSSIE_HOME)/lib/python -b ossie.omnijni.idljava')
  _RH_SILENT_COMMAND([IDLJNI], '@echo "  IDLJNI" $<;')
])

dnl _RH_PROG_IDLJNICXX (for internal use only)
AC_DEFUN([_RH_PROG_IDLJNICXX],
[
  AC_SUBST([IDLJNICXX], '$(IDL) -p $(OSSIE_HOME)/lib/python -b ossie.omnijni.idljni')
])

dnl RH_IDLJ_RULES
dnl Sets up requirements for generating Java source from IDL using the distributed
dnl makefile fragment
AC_DEFUN([RH_IDLJ_RULES],
[
  AC_REQUIRE([RH_PROG_IDLJ])
  AC_REQUIRE([_RH_PROG_IDLJNI])
  AC_REQUIRE([_RH_AUTOMAKE_TARGETS])

  dnl Set a variable to point to the makefile fragment, which can then be used
  dnl in place of the actual path
  AC_SUBST([rh_idlj_rules], 'include $(OSSIE_HOME)/share/aminclude/redhawk/idlj.am')
])

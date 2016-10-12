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
])

dnl RH_PROG_IDLJ
AC_DEFUN([RH_PROG_IDLJ],
[
  AC_REQUIRE([RH_JAVA_HOME])
  java_test_paths=$JAVA_HOME/jre/sh$PATH_SEPARATOR$JAVA_HOME/bin
  AC_PATH_PROG([IDLJ], [idlj], [no], [$java_test_paths])
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
AC_DEFUN([RH_HAVA_JNI_H],
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


/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK bulkioInterfaces.
 *
 * REDHAWK bulkioInterfaces is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK bulkioInterfaces is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */
AC_INIT(Java_Ports, 1.0.0)
AM_INIT_AUTOMAKE(nostdinc)

AC_CORBA_ORB
OSSIE_CHECK_OSSIE
OSSIE_SDRROOT_AS_PREFIX

export PKG_CONFIG_PATH="$PKG_CONFIG_PATH:/usr/local/lib/pkgconfig"
PKG_CHECK_MODULES([OSSIE], [ossie >= 1.8])

AC_CHECK_PROG([IDLJ], [idlj], [idlj], [AC_MSG_ERROR([cannot find idlj program])])
AC_CHECK_PROG([JAVAC], [javac], [javac], [AC_MSG_ERROR([cannot find Java compiler])])
AC_CHECK_PROG([JAR], [jar], [jar], [AC_MSG_ERROR([cannot find jar program])])

AC_PATH_PROG(JAVAC, javac, [not found], [${JAVA_HOME}/bin])
AC_PATH_PROG(JAVAH, javah, [not found], [${JAVA_HOME}/bin])
AC_PATH_PROG(JAVA, java, [not found], [${JAVA_HOME}/bin])
AC_PATH_PROG(JAR, jar, [not found], [${JAVA_HOME}/bin])
AC_SUBST(JAVAC)
AC_SUBST(JAVAH)
AC_SUBST(JAR)
AC_SUBST(JAVA)

export PKG_CONFIG_PATH="../../../..:../../..":$PKG_CONFIG_PATH
PKG_CHECK_MODULES([BIO], [bulkio >= 1.10])

AC_CONFIG_FILES(Makefile)
AC_OUTPUT

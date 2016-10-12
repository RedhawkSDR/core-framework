AC_INIT(Oversized_framedata, 1.0.0)
AM_INIT_AUTOMAKE(nostdinc)

AC_CORBA_ORB
OSSIE_CHECK_OSSIE
OSSIE_SDRROOT_AS_PREFIX

export PKG_CONFIG_PATH="$PKG_CONFIG_PATH:/usr/local/lib/pkgconfig"
CLASSPATH_SOFTPKG_DEP=""
AC_SUBST(CLASSPATH_SOFTPKG_DEP)
PKG_CHECK_MODULES([OSSIE], [ossie >= 1.10])

AC_CHECK_PROG([JAVAC], [javac], [javac], [AC_MSG_ERROR([cannot find Java compiler])])
AC_CHECK_PROG([JAR], [jar], [jar], [AC_MSG_ERROR([cannot find jar program])])

PKG_CHECK_MODULES([INTERFACEDEPS], [bulkio >= 1.10])

AC_CONFIG_FILES(Makefile)
AC_OUTPUT

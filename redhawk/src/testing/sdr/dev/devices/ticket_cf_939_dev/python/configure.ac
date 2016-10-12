
AC_INIT(python, 1.0.0)
AM_INIT_AUTOMAKE(nostdinc)

AC_PROG_INSTALL

AC_CORBA_ORB
OSSIE_CHECK_OSSIE
OSSIE_SDRROOT_AS_PREFIX
AM_PATH_PYTHON([2.3])

PKG_CHECK_MODULES([OSSIE], [ossie >= 1.8])
AC_CHECK_PYMODULE(ossie, [], [AC_MSG_ERROR([the python ossie module is required])])
PKG_CHECK_MODULES([OMNIORB], [omniORB4 >= 4.0.0])
AC_CHECK_PYMODULE(omniORB, [], [AC_MSG_ERROR([the python omniORB module is required])])


AC_CONFIG_FILES(Makefile)

AC_OUTPUT

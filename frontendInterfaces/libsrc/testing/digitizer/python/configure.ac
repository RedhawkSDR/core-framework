AC_INIT(digitizer, 1.0.0)
AM_INIT_AUTOMAKE([nostdinc foreign])
AC_CONFIG_MACRO_DIR([m4])

AC_PROG_INSTALL

AC_CORBA_ORB
OSSIE_CHECK_OSSIE
OSSIE_SDRROOT_AS_PREFIX
AM_PATH_PYTHON([2.6])

PKG_CHECK_MODULES([OSSIE], [ossie >= 3.0])
AC_CHECK_PYMODULE(ossie, [], [AC_MSG_ERROR([the python ossie module is required])])
PKG_CHECK_MODULES([OMNIORB], [omniORB4 >= 4.1.0])
AC_CHECK_PYMODULE(omniORB, [], [AC_MSG_ERROR([the python omniORB module is required])])

PKG_CHECK_MODULES(frontend, frontend >= 3.0) 
AC_CHECK_PYMODULE(frontend, [], [AC_MSG_ERROR([the python frontend module is required])])
PKG_CHECK_MODULES(bulkio, bulkio >= 3.0) 
AC_CHECK_PYMODULE(bulkio, [], [AC_MSG_ERROR([the python bulkio module is required])])

AC_CONFIG_FILES(Makefile)

AC_OUTPUT

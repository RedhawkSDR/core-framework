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

dnl Let the user specify OSSIEHOME.  The priority is
dnl 1. the --with-ossie argument
dnl 2. the OSSIEHOME environment variable
dnl 3. the --prefix argument
AC_DEFUN([OSSIE_OSSIEHOME],
[
  AC_CACHE_CHECK([for ossie home], ossie_cv_ossie_home,
  [
    AC_ARG_WITH(ossie,
      AC_HELP_STRING([--with-ossie], [ossie root directory (default from environment variable 'OSSIEHOME', otherwise from prefix)]),
      ossie_cv_ossie_home=$withval,
      if test "x${OSSIEHOME}" != "x"; then
        ossie_cv_ossie_home=${OSSIEHOME}
      elif test "x${prefix}" != "xNONE"; then
        ossie_cv_ossie_home=${prefix}
      else
        ossie_cv_ossie_home=$ac_default_prefix
      fi
    )

    dnl Check if this is a cross, if so prepend the sysroot to the ossie home
    AS_IF([test "x$cross_compiling" = "xyes"], [
      CROSS_SYSROOT=`$CC --print-sysroot`
      ossie_cv_ossie_home=${CROSS_SYSROOT}${ossie_cv_ossie_home}
    ])
  ])
  AC_SUBST(OSSIE_HOME, $ossie_cv_ossie_home)
])

dnl Check that OSSIE is installed so we can compile against it
AC_DEFUN([OSSIE_CHECK_OSSIE],
[
AC_REQUIRE([OSSIE_OSSIEHOME])
AC_MSG_CHECKING([to see ossie is installed])
if test -e $ossie_cv_ossie_home/lib64; then
  PKG_CONFIG_PATH="${ossie_cv_ossie_home}/lib${gr_libdir_suffix}/pkgconfig:${ossie_cv_ossie_home}/lib64${gr_libdir_suffix}/pkgconfig:${PKG_CONFIG_PATH}"
else
  PKG_CONFIG_PATH="${ossie_cv_ossie_home}/lib${gr_libdir_suffix}/pkgconfig:${PKG_CONFIG_PATH}"
fi
export PKG_CONFIG_PATH
])

dnl use OSSIEHOME as the default prefix unless --prefix is provided
AC_DEFUN([OSSIE_OSSIEHOME_AS_PREFIX],
[
  AS_IF([test "x${prefix}" = "xNONE"], [
    dnl Prefix wasn't provided, we need to use ossie home
    AC_REQUIRE([OSSIE_OSSIEHOME])
    AS_IF([test "x${ossie_cv_ossie_home}" = "xNONE"], [
      AC_MSG_ERROR([ossie root directory is not set; this is not expected])
    ])
    dnl Use ossie home value for prefix
    ac_default_prefix=${ossie_cv_ossie_home}
    prefix=${ossie_cv_ossie_home}
    AC_MSG_NOTICE(using ${ossie_cv_ossie_home} as installation prefix)
  ])
])

dnl A variant on OSSIE_SDRROOT for use *only* when OSSIE_OSSIEHOME_AS_PREFIX is being used. Priorities:
dnl 1. the --with-sdr argument
dnl 2. the SDRROOT environment variable
dnl 3. the default SDRROOT directory (/var/redhawk/sdr)
AC_DEFUN([OSSIE_SDRROOT_IGNORE_PREFIX],
[
  AC_CACHE_CHECK([for sdr root], ossie_cv_sdr_root,
  [
    AC_ARG_WITH(sdr,
      AC_HELP_STRING([--with-sdr], [sdr root directory (default from environment variable 'SDRROOT', otherwise /var/redhawk/sdr)]),
      ossie_cv_sdr_root=$withval,
      AS_IF([test "x${SDRROOT}" != "x"], [ossie_cv_sdr_root=${SDRROOT}],
            [ossie_cv_sdr_root=/var/redhawk/sdr]))
  ])
  AC_SUBST(SDR_ROOT, $ossie_cv_sdr_root)
])

dnl Let the user specify SDRROOT.  The priority is
dnl 1. the --with-sdr argument
dnl 2. the SDRROOT environment variable
dnl 3. the --prefix argument
AC_DEFUN([OSSIE_SDRROOT],
[
  AC_CACHE_CHECK([for sdr root], ossie_cv_sdr_root,
  [
    AC_ARG_WITH(sdr,
      AC_HELP_STRING([--with-sdr], [sdr root directory (default from environment variable 'SDRROOT', otherwise from prefix)]),
      ossie_cv_sdr_root=$withval,
      if test "x${SDRROOT}" != "x"; then
        ossie_cv_sdr_root=${SDRROOT}
      elif test "x${prefix}" != "xNONE"; then
        ossie_cv_sdr_root=${prefix}
      else
        ossie_cv_sdr_root=/var/redhawk/sdr
      fi
    )
  ])
  AC_SUBST(SDR_ROOT, $ossie_cv_sdr_root)
])

dnl use SDRROOT as the default prefix unless --prefix is provided
AC_DEFUN([OSSIE_SDRROOT_AS_PREFIX],
[
  AS_IF([test "x${prefix}" = "xNONE"],
  [
    dnl Prefix wasn't provided, we need to use sdr root
    AC_REQUIRE([OSSIE_SDRROOT])
    AS_IF([test "x${ossie_cv_sdr_root}" = "xNONE"], [
      AC_MSG_ERROR([sdr root directory is not set; this is not expected])
    ])
    dnl Use sdr root value, suffixed with any arg to this macro, for the prefix
    ac_default_prefix=${ossie_cv_sdr_root}/$1
    prefix=${ossie_cv_sdr_root}/$1
    AC_MSG_NOTICE(using ${ossie_cv_sdr_root}/$1 as installation prefix)
  ])
])

dnl other misc functions
AC_DEFUN([OSSIE_ENABLE_TRACE],
[AC_MSG_CHECKING([to see if tracing should be enabled])
AC_ARG_ENABLE(trace, AS_HELP_STRING([--enable-trace], [Enable LOG_TRACE statements]) , WITH_TRACE=yes ; AC_DEFINE(ENABLE_TRACE), WITH_TRACE=no)
AC_MSG_RESULT($WITH_TRACE)
AC_SUBST(WITH_TRACE)
])

AC_DEFUN([OSSIE_ENABLE_LOG4CXX],
[AC_ARG_ENABLE(log4cxx, AS_HELP_STRING([--disable-log4cxx], [Disable log4cxx support]))
if test "x$enable_log4cxx" != "xno"; then
  WITH_LOG4CXX=yes
  PKG_CHECK_MODULES([LOG4CXX], [liblog4cxx >= 0.10.0]) 
  LIBS="$LIBS $LOG4CXX_LIBS"
  CPPFLAGS="${CPPFLAGS} ${LOG4CXX_CFLAGS}"
  AC_DEFINE(HAVE_LOG4CXX) 
  AC_DEFINE(RH_LOGGER)
  AC_SUBST(HAVE_LOG4CXX,[-DHAVE_LOG4CXX=1]) 
  AC_SUBST(LOG4CXX_LIBS, $LOG4CXX_LIBS) 
  AC_SUBST(LOG4CXX_CFLAGS, $LOG4CXX_CFLAGS) 
else
  WITH_LOG4CXX=no
  AC_DEFINE(RH_LOGGER)
  CPPFLAGS="${CPPFLAGS} -DRH_LOGGER"
  AC_SUBST(HAVE_LOG4CXX) 
  AC_SUBST(LOG4CXX_LIBS) 
  AC_SUBST(LOG4CXX_CFLAGS) 
fi 
  AC_SUBST(WITH_LOG4CXX)
])

AC_DEFUN([OSSIE_PYTHON_INSTALL_SCHEME],
[AC_CACHE_CHECK(for python install scheme,
ossie_cv_pyscheme,
[AC_ARG_WITH(pyscheme,
             AC_HELP_STRING([--with-pyscheme], [python scheme, either 'prefix' or 'home']),
             ossie_cv_pyscheme=$withval,
             [if test "${prefix}" = "/usr" -o "${prefix}" = "/usr/local" -o "${prefix}" = "NONE"; then
                ossie_cv_pyscheme="prefix"
              else
                ossie_cv_pyscheme="home"
              fi
             ])
])
AC_SUBST(PYTHON_INSTALL_SCHEME, $ossie_cv_pyscheme)
])
  
AC_DEFUN([OSSIE_ENABLE_PERSISTENCE], [
  AC_MSG_CHECKING([to see if domain persistence should be enabled])
  AC_ARG_ENABLE(persistence, [
AS_HELP_STRING([--enable-persistence@<:@=persist_type@:>@], [Enable persistence support.  Supported types: bdb, gdbm, sqlite @<:@default=sqlite@:>@])
AS_HELP_STRING([--disable-persistence], [Disable persistence support])],
                [],
                [
                dnl Default behavior is implicit yes
                enable_persistence="yes"
                ])

  dnl Default backend is sqlite
  AS_IF([test "x$enable_persistence" = "xyes"], [
    enable_persistence="sqlite"
  ])

  AS_IF([test "x$enable_persistence" = "xno" -o "x$enable_persistence" = "xnone"], [
    AC_MSG_RESULT([no])
  ], [
    AC_MSG_RESULT([$enable_persistence])
    if test "x$enable_persistence" = "xsqlite"; then
        CHECK_SQLITE3_LIB
        if test x"$ac_sqlite3_header" == "xyes" -a  x"$ac_sqlite3_lib" == "xyes"; then
          PERSISTENCE_CFLAGS=""
          PERSISTENCE_LIBS="-lsqlite3"
          AC_DEFINE(HAVE_SQLITE, 1, [Define if sqlite is available])
	  AC_DEFINE(ENABLE_SQLITE_PERSISTENCE, 1, [enable SQLite-based persistence])
        else
	  AC_MSG_ERROR([System cannot support sqlite persistence])
        fi
    elif test "x$enableval" == "xbdb"; then
	CHECK_BDB_LIB
        if test x"$ac_bdb_header" == "xyes" -a  x"$ac_bdb_lib" == "xyes"; then
          PERSISTENCE_CFLAGS=""
          PERSISTENCE_LIBS="-ldb_cxx"
          AC_DEFINE(HAVE_BDB, 1, [Define if bdb is available])
	  AC_DEFINE(ENABLE_BDB_PERSISTENCE, 1, [enable BDB-based persistence])
        else
	  AC_MSG_ERROR([System cannot support bdb persistence])
        fi
    elif test "x$enableval" == "xgdbm"; then
	CHECK_GDBM_LIB
        if test x"$ac_gdbm_header" == "xyes" -a  x"$ac_gdbm_lib" == "xyes"; then
          PERSISTENCE_CFLAGS=""
          PERSISTENCE_LIBS="-lgdbm"
          AC_DEFINE(HAVE_GDBM, 1, [Define if gdbm is available])
	  AC_DEFINE(ENABLE_GDBM_PERSISTENCE, 1, [enable gdbm-based persistence])
        else
	  AC_MSG_ERROR([System cannot support gdbm persistence])
        fi
    else
	AC_MSG_ERROR([Invalid persistence type specified])
    fi
    AC_SUBST(PERSISTENCE_CFLAGS)
    AC_SUBST(PERSISTENCE_LIBS)
  ])
])

AC_DEFUN([CHECK_BDB_LIB],
[
AC_LANG_PUSH([C++])
AC_CHECK_HEADER([db4/db_cxx.h], [ac_bdb_header="yes"], [ac_bdb_header="no"])
dnl Check thd db_cxx library
AC_MSG_CHECKING([for Db in -ldb_cxx])
SAVED_LDFLAGS=$LDFLAGS
LDFLAGS="$LDFLAGS -ldb_cxx"
AC_LINK_IFELSE(
  [AC_LANG_PROGRAM([#include <db4/db_cxx.h>],
   [Db test(0, 0);])
  ],
  [ ac_bdb_lib="yes"], [ac_bdb_lib="no"])
AC_MSG_RESULT($ac_bdb_lib)
AC_LANG_POP([C++])
LDFLAGS="$SAVED_LDFLAGS"

])

AC_DEFUN([CHECK_SQLITE3_LIB],
[
AC_CHECK_HEADER([sqlite3.h], [ac_sqlite3_header="yes"], [ac_sqlite3_header="no"])
AC_CHECK_LIB([sqlite3], [sqlite3_open], [ac_sqlite3_lib="yes"], [ac_sqlite3_lib="no"])
])

AC_DEFUN([CHECK_GDBM_LIB],
[
AC_CHECK_HEADER([gdbm.h], [ac_gdbm_header="yes"], [ac_gdbm_header="no"])
AC_CHECK_LIB([gdbm], [gdbm_open], [ac_gdbm_lib="yes"], [ac_gdbm_lib="no"]) 
])


AC_DEFUN([CHECK_VECTOR_IMPL],
[AC_CACHE_CHECK(vector implementation,
ac_cv_vector_impl,
[AC_LANG_SAVE
 AC_LANG_CPLUSPLUS
 AC_TRY_COMPILE([
    #include <vector>
    #include <cstring>
 ], [
    char* ptr = new char[22];
    strcpy(ptr,"I WANT TO BE A VECTOR");
    std::vector<char> charVector;
    std::_Vector_base<char, std::allocator<char> >::_Vector_impl *vectorPointer = (std::_Vector_base<char, std::allocator<char> >::_Vector_impl *) ((void*) & charVector);
    vectorPointer->_M_start = (char*) &(ptr[0]);
    vectorPointer->_M_finish = vectorPointer->_M_start + 22;
    vectorPointer->_M_end_of_storage = vectorPointer->_M_finish;
    return 0;
 ],
 ac_cv_vector_impl=yes,
 ac_cv_vector_impl=no)
 AC_LANG_RESTORE
])
if test "$ac_cv_vector_impl" = yes; then
  AC_DEFINE(EXPECTED_VECTOR_IMPL,,[define checks for a specific implementation of vector])
fi
])

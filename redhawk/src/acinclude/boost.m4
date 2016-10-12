# ===========================================================================
#             http://autoconf-archive.cryp.to/ax_boost_base.html
# ===========================================================================
#
# SYNOPSIS
#
#   AX_BOOST_BASE([MINIMUM-VERSION])
#
# DESCRIPTION
#
#   Test for the Boost C++ libraries of a particular version (or newer)
#
#   If no path to the installed boost library is given the macro searchs
#   under /usr, /usr/local, /opt and /opt/local and evaluates the
#   $BOOST_ROOT environment variable. Further documentation is available at
#   <http://randspringer.de/boost/index.html>.
#
#   This macro calls:
#
#     AC_SUBST(BOOST_CPPFLAGS) / AC_SUBST(BOOST_LDFLAGS)
#
#   And sets:
#
#     HAVE_BOOST
#
# LAST MODIFICATION
#
#   2008-04-12
#
# COPYLEFT
#
#   Copyright (c) 2008 Thomas Porschberg <thomas@randspringer.de>
#   Copyright (c) 2008 Free Software Foundation, Inc.
#
#   Copying and distribution of this file, with or without modification, are
#   permitted in any medium without royalty provided the copyright notice
#   and this notice are preserved.

AC_DEFUN([AX_BOOST_BASE],
[
AC_REQUIRE([GR_LIB64])
AC_ARG_WITH([boost],
    AS_HELP_STRING([--with-boost@<:@=DIR@:>@],
		   [use boost (default is yes) - it is possible to specify the root directory for boost (optional)]),
    [
    if test "$withval" = "no"; then
        want_boost="no"
    elif test "$withval" = "yes"; then
        want_boost="yes"
        ac_boost_path=""
    else
        want_boost="yes"
        ac_boost_path="$withval"
    fi
    ],
    [want_boost="yes"])


AC_ARG_WITH([boost-libdir],
        AS_HELP_STRING([--with-boost-libdir=LIB_DIR],
		       [Force given directory for boost libraries. Note that this
		        will overwrite library path detection, so use this parameter
		        only if default library detection fails and you know exactly
                        where your boost libraries are located.]),
        [
        if test -d $withval
        then
                ac_boost_lib_path="$withval"
        else
                AC_MSG_ERROR(--with-boost-libdir expected directory name)
        fi
        ],
        [ac_boost_lib_path=""]
)

if test "x$want_boost" = "xyes"; then
    boost_lib_version_req=ifelse([$1], ,1.20.0,$1)
    boost_lib_version_req_shorten=`expr $boost_lib_version_req : '\([[0-9]]*\.[[0-9]]*\)'`
    boost_lib_version_req_major=`expr $boost_lib_version_req : '\([[0-9]]*\)'`
    boost_lib_version_req_minor=`expr $boost_lib_version_req : '[[0-9]]*\.\([[0-9]]*\)'`
    boost_lib_version_req_sub_minor=`expr $boost_lib_version_req : '[[0-9]]*\.[[0-9]]*\.\([[0-9]]*\)'`
    if test "x$boost_lib_version_req_sub_minor" = "x" ; then
        boost_lib_version_req_sub_minor="0"
        fi
    WANT_BOOST_VERSION=`expr $boost_lib_version_req_major \* 100000 \+  $boost_lib_version_req_minor \* 100 \+ $boost_lib_version_req_sub_minor`
    AC_MSG_CHECKING(for boost >= $boost_lib_version_req)
    succeeded=no

    dnl first we check the system location for boost libraries
    dnl this location ist chosen if boost libraries are installed with the --layout=system option
    dnl or if you install boost with RPM
    if test "$ac_boost_path" != ""; then
	dnl Look first where we think they ought to be, accounting for a possible "64" suffix on lib.
	dnl If that directory doesn't exist, fall back to the default behavior
	if test -d "$ac_boost_path/lib${gr_libdir_suffix}"; then
            BOOST_LDFLAGS="-L$ac_boost_path/lib${gr_libdir_suffix}"
        else
            BOOST_LDFLAGS="-L$ac_boost_path/lib"
        fi
        BOOST_CPPFLAGS="-I$ac_boost_path/include"
    else
        dnl Search in the users LD_LIBRARY_PATH first, making an assumption
	dnl about the 
        ld_library_path_tmp=`echo $LD_LIBRARY_PATH | tr ':' ' '`
        for ac_boost_path_tmp in $ld_library_path_tmp  ; do
            if test -d "${ac_boost_path_tmp%/lib${gr_libdir_suffix}}/include/boost" && test -r "${ac_boost_path_tmp%/lib${gr_libdir_suffix}}/include/boost"; then
		dnl Look first where we think they ought to be, accounting for a possible "64" suffix on lib.
		dnl If that directory doesn't exist, fall back to the default behavior
		if test -d "$ac_boost_path_tmp"; then
                    BOOST_LDFLAGS="-L$ac_boost_path_tmp"
		else
	            BOOST_LDFLAGS="-L$ac_boost_path_tmp"
		fi
                BOOST_CPPFLAGS="-I${ac_boost_path_tmp%/lib${gr_libdir_suffix}}/include"
                break;
            fi
        done

	if test "x$LIBTOOL_SYSROOT_PATH" != "x"; then
          ac_boost_sysroot_path=$LIBTOOL_SYSROOT_PATH 
        else
          ac_boost_sysroot_path=""
        fi

        if test "x${BOOST_LDFLAGS}" = "x"; then
	    dnl Now look in standard locations
	    for ac_boost_path_tmp in /usr/local /usr /opt /opt/local ; do
                ac_boost_path_tmp=$ac_boost_sysroot_path$ac_boost_path_tmp
                echo $ac_boost_path_tmp
		if test -d "$ac_boost_path_tmp/include/boost" && test -r "$ac_boost_path_tmp/include/boost"; then
		    dnl Look first where we think they ought to be, accounting for a possible "64" suffix on lib.
		    dnl If that directory doesn't exist, fall back to the default behavior
		    if test -d "$ac_boost_path_tmp/lib${gr_libdir_suffix}"; then
			BOOST_LDFLAGS="-L$ac_boost_path_tmp/lib${gr_libdir_suffix}"
		    else
			BOOST_LDFLAGS="-L$ac_boost_path_tmp/lib"
		    fi
		    BOOST_CPPFLAGS="-I$ac_boost_path_tmp/include"
		    break;
		fi
	    done
	fi
    fi

    dnl overwrite ld flags if we have required special directory with
    dnl --with-boost-libdir parameter
    if test "$ac_boost_lib_path" != ""; then
       BOOST_LDFLAGS="-L$ac_boost_lib_path"
    fi

    CPPFLAGS_SAVED="$CPPFLAGS"
    CPPFLAGS="$CPPFLAGS $BOOST_CPPFLAGS"
    export CPPFLAGS

    LDFLAGS_SAVED="$LDFLAGS"
    LDFLAGS="$LDFLAGS $BOOST_LDFLAGS"
    export LDFLAGS

    AC_LANG_PUSH(C++)
        AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
    @%:@include <boost/version.hpp>
    ]], [[
    #if BOOST_VERSION >= $WANT_BOOST_VERSION
    // Everything is okay
    #else
    #  error Boost version is too old
    #endif
    ]])],[AC_MSG_RESULT(yes)
	  succeeded=yes
	  found_system=yes
          ],
         [])
    AC_LANG_POP([C++])
    CPPFLAGS="$CPPFLAGS_SAVED"
    LDFLAGS="$LDFLAGS_SAVED"


    dnl if we found no boost with system layout we search for boost libraries
    dnl built and installed without the --layout=system option
    if test "$succeeded" != "yes"; then
        _version=0

        if test "$ac_boost_path" != ""; then
	    path_list="$ac_boost_path"
	else
	    path_list="/usr /usr/local /opt /opt/local"
	fi
        for ac_boost_path in $path_list ; do
	    if test -d "$ac_boost_path" && test -r "$ac_boost_path"; then
            	for i in `ls -d $ac_boost_path/include/boost-* 2>/dev/null`; do
		    _version_tmp=`echo $i | sed "s#$ac_boost_path##" | sed 's,/include/boost-,,; s,_,.,'`
                    V_CHECK=`expr $_version_tmp \> $_version`
                    if test "$V_CHECK" = "1" ; then
                        _version=$_version_tmp
                        best_path=$ac_boost_path
		    fi
                done
            fi
	done

        VERSION_UNDERSCORE=`echo $_version | sed 's/\./_/'`
        BOOST_CPPFLAGS="-I$best_path/include/boost-$VERSION_UNDERSCORE"

        if test "$ac_boost_lib_path" = "";  then
	    dnl Look first where we think they ought to be, accounting for a possible "64" suffix on lib.
	    dnl If that directory doesn't exist, fall back to the default behavior
	    if test -d "$best_path/lib${gr_libdir_suffix}"; then
                BOOST_LDFLAGS="-L$best_path/lib${gr_libdir_suffix}"
	    else
                BOOST_LDFLAGS="-L$best_path/lib"
	    fi
        fi

        CPPFLAGS="$CPPFLAGS $BOOST_CPPFLAGS"
        export CPPFLAGS
        LDFLAGS="$LDFLAGS $BOOST_LDFLAGS"
        export LDFLAGS

        AC_LANG_PUSH(C++)
            AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
        @%:@include <boost/version.hpp>
        ]], [[
        #if BOOST_VERSION >= $WANT_BOOST_VERSION
        // Everything is okay
        #else
        #  error Boost version is too old
        #endif
        ]])],[AC_MSG_RESULT(yes)
	      succeeded=yes
              found_system=yes
              ],
	     [])
        AC_LANG_POP([C++])
        CPPFLAGS="$CPPFLAGS_SAVED"
        LDFLAGS="$LDFLAGS_SAVED"
    fi

    if test "$succeeded" != "yes" ; then
	AC_MSG_RESULT([no])
        if test "$_version" = "0" ; then
            AC_MSG_ERROR([[we could not detect the boost libraries (version $boost_lib_version_req_shorten or higher).
If you are sure you have boost installed, then check your version number looking in <boost/version.hpp>.]])
        else
            AC_MSG_ERROR([your boost libraries seem to old (version $_version).])
        fi
    else
        AC_SUBST(BOOST_CPPFLAGS)
        AC_SUBST(BOOST_LDFLAGS)
        AC_DEFINE(HAVE_BOOST,1,[Define if the Boost headers are available])
    fi
fi
])

dnl
dnl Macros used by the boost items that need libraries.
dnl

dnl $1 is unit name.  E.g., boost_thread
AC_DEFUN([_AX_BOOST_CHECK_LIB],[
    _AX_BOOST_CHECK_LIB_($1,HAVE_[]m4_toupper($1),m4_toupper($1)_LIB)
])

dnl $1 is unit name.  E.g., boost_thread
dnl $2 is AC_DEFINE name.  E.g., HAVE_BOOST_THREAD
dnl $3 is lib var name.    E.g., BOOST_THREAD_LIB
AC_DEFUN([_AX_BOOST_CHECK_LIB_],[
    AC_LANG_PUSH([C++])
    AC_DEFINE($2,1,[Define if the $1 library is available])
    BOOSTLIBDIR=`echo $BOOST_LDFLAGS | sed -e 's/@<:@^\/@:>@*//'`

    dnl See if we can find a usable library
    link_ok="no"
    if test "$ax_boost_user_lib" != ""; then
        dnl use what the user supplied 
        for ax_lib in $ax_boost_user_lib $1-${ax_boost_user_lib}; do
	    AC_CHECK_LIB($ax_lib, exit,
                         [$3="-l$ax_lib"; AC_SUBST($3) link_ok="yes"; break])
        done
    else
	dnl Look in BOOSTLIBDIR for possible candidates
	head=$BOOSTLIBDIR/lib[]$1
	for f in ${head}*.so* ${head}*.a* ${head}*.dll* ${head}*.dylib; do
	    dnl echo 1: $f
	    case $f in
	      *\**) continue;;
	    esac
	    f=`echo $f | sed -e 's,.*/,,' -e 's,^lib,,'`
	    dnl echo 2: $f
	    f=`echo $f | sed -e 's,\($1.*\)\.so.*$,\1,' -e 's,\($1.*\)\.a.*$,\1,' -e 's,\($1.*\)\.dll.*$,\1,' -e 's,\($1.*\)\.dylib.*$,\1,'`
	    dnl echo 3: $f

	    ax_lib=$f
            AC_CHECK_LIB($ax_lib, exit,
                        [$3="-l$ax_lib"; AC_SUBST($3) link_ok="yes"; break])
	done
    fi		    
		    		    
    if test "$link_ok" != "yes"; then
    	AC_MSG_ERROR([Could not link against lib[$1]!])
    fi
    AC_LANG_POP([C++])
])


dnl $1 is unit name.  E.g., boost_thread
AC_DEFUN([_AX_BOOST_WITH],[
    _AX_BOOST_WITH_($1,m4_bpatsubst($1,_,-))
])

dnl $1 is unit name.  E.g., boost_thread
dnl $2 is hyphenated unit name.  E.g., boost-thread
AC_DEFUN([_AX_BOOST_WITH_],[
    AC_ARG_WITH([$2],
    		AC_HELP_STRING([--with-$2@<:@=special-lib@:>@],
		               [Use the m4_substr($1,6) library from boost.  It is possible to specify a certain
		                library to the linker.  E.g., --with-$2=$1-gcc41-mt-1_35]),
        	[
	        if test "$withval" = "no"; then
	            want_boost="no"
	        elif test "$withval" = "yes"; then
	            want_boost="yes"
	            ax_boost_user_lib=""
	        else
	            want_boost="yes"
	            ax_boost_user_lib="$withval"
	        fi
	        ],
	        [want_boost="yes"])
])

dnl $1 is unit name.  E.g., boost_thread
dnl $2 is AC_LANG_PROGRAM argument 1
dnl $3 is AC_LANG_PROGRAM argument 2
dnl $4 is cv variable name.  E.g., ax_cv_boost_thread
AC_DEFUN([_AX_BOOST_CHECK_],[
    _AX_BOOST_WITH($1)
    if test "$want_boost" = "yes"; then
        AC_REQUIRE([AC_PROG_CC])
        AC_REQUIRE([AC_PROG_CXX])
        CPPFLAGS_SAVED="$CPPFLAGS"
        CPPFLAGS="$CPPFLAGS $BOOST_CPPFLAGS"
        LDFLAGS_SAVED="$LDFLAGS"
        LDFLAGS="$LDFLAGS $BOOST_LDFLAGS"
        AC_CACHE_CHECK([whether the boost::m4_substr([$1],6) includes are available], [$4],
		       [AC_LANG_PUSH([C++])
                        AC_COMPILE_IFELSE(AC_LANG_PROGRAM([$2],[$3]),[$4]=yes,[$4]=no)
                        AC_LANG_POP([C++])
                       ])
	if test "$[$4]" = "yes"; then
	    _AX_BOOST_CHECK_LIB([$1])
	fi
        CPPFLAGS="$CPPFLAGS_SAVED"
        LDFLAGS="$LDFLAGS_SAVED"
    fi
])

dnl $1 is unit name.  E.g., boost_thread
dnl $2 is AC_LANG_PROGRAM argument 1
dnl $3 is AC_LANG_PROGRAM argument 2
AC_DEFUN([_AX_BOOST_CHECK],[
    _AX_BOOST_CHECK_($1,$2,$3,ax_cv_$1)
])

# ===========================================================================
#       http://www.nongnu.org/autoconf-archive/ax_boost_filesystem.html
# ===========================================================================
#
# SYNOPSIS
#
#   AX_BOOST_FILESYSTEM
#
# DESCRIPTION
#
#   Test for Filesystem library from the Boost C++ libraries. The macro
#   requires a preceding call to AX_BOOST_BASE. Further documentation is
#   available at <http://randspringer.de/boost/index.html>.
#
#   This macro calls:
#
#     AC_SUBST(BOOST_FILESYSTEM_LIB)
#
#   And sets:
#
#     HAVE_BOOST_FILESYSTEM
#
# LICENSE
#
#   Copyright (c) 2009 Thomas Porschberg <thomas@randspringer.de>
#   Copyright (c) 2009 Michael Tindal
#   Copyright (c) 2009 Roman Rybalko <libtorrent@romanr.info>
#
#   Copying and distribution of this file, with or without modification, are
#   permitted in any medium without royalty provided the copyright notice
#   and this notice are preserved.
AC_DEFUN([_BOOST_CHECK_LIB],
[
dnl $1 is the library
dnl $2 is the header
dnl $3 is the cxx code
dnl $4 action if pass
dnl $5 action if not pass

AC_LANG_PUSH([C++])
dnl Check thd db_cxx library
AC_MSG_CHECKING([for -l$1])
SAVED_LIBS=$LIBS
LIBS="$LIBS -l$1"
AC_LINK_IFELSE(
  [AC_LANG_PROGRAM([#include <$2>],
   [$3])
  ],
  [AC_MSG_RESULT(yes); $4], 
  [AC_MSG_RESULT(no); $5])
AC_LANG_POP([C++])
LIBS="$SAVED_LIBS"

dnl if test x"$ac_bdb_header" == "xyes" -a  x"$ac_bdb_lib" == "xyes"; then
dnl   AC_SUBST(BDB_CFLAGS, "")
dnl   AC_SUBST(BDB_LIBS, "-ldb_cxx")
dnl   AC_DEFINE(HAVE_BDB, 1, [Define if bdb is available])
dnl else
dnl   AC_SUBST(BDB_CFLAGS, "")
dnl   AC_SUBST(BDB_LIBS, "")
dnl   AC_DEFINE(HAVE_BDB, 0, [Define if bdb is available])
dnl fi
])

AC_DEFUN([AX_BOOST_FILESYSTEM],
[
	AC_ARG_WITH([boost-filesystem],
	AS_HELP_STRING([--with-boost-filesystem@<:@=special-lib@:>@],
                   [use the Filesystem library from boost - it is possible to specify a certain library for the linker
                        e.g. --with-boost-filesystem=boost_filesystem-gcc-mt ]),
        [
        if test "$withval" = "no"; then
			want_boost="no"
        elif test "$withval" = "yes"; then
            want_boost="yes"
            ax_boost_user_filesystem_lib=""
        else
		    want_boost="yes"
        	ax_boost_user_filesystem_lib="$withval"
		fi
        ],
        [want_boost="yes"]
	)

	if test "x$want_boost" = "xyes"; then
        AC_REQUIRE([AC_PROG_CC])
		CPPFLAGS_SAVED="$CPPFLAGS"
		CPPFLAGS="$CPPFLAGS $BOOST_CPPFLAGS"
		export CPPFLAGS

		LDFLAGS_SAVED="$LDFLAGS"
		LDFLAGS="$LDFLAGS $BOOST_LDFLAGS"
		export LDFLAGS

		LIBS_SAVED=$LIBS
		LIBS="$LIBS $BOOST_SYSTEM_LIB"
		export LIBS

        AC_CACHE_CHECK(whether the Boost::Filesystem library is available,
					   ax_cv_boost_filesystem,
        [AC_LANG_PUSH([C++])
         AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[@%:@include <boost/filesystem/path.hpp>]],
                                   [[using namespace boost::filesystem;
                                   path my_path( "foo/bar/data.txt" );
                                   return 0;]])],
            				       ax_cv_boost_filesystem=yes, ax_cv_boost_filesystem=no)
         AC_LANG_POP([C++])
		])
		if test "x$ax_cv_boost_filesystem" = "xyes"; then
			AC_DEFINE(HAVE_BOOST_FILESYSTEM,,[define if the Boost::Filesystem library is available])
            BOOSTLIBDIR=`echo $BOOST_LDFLAGS | sed -e 's/@<:@^\/@:>@*//'`
            if test "x$ax_boost_user_filesystem_lib" = "x"; then
                for libextension in `ls $BOOSTLIBDIR/libboost_filesystem*.{so,dylib,a}* 2>/dev/null | sed 's,.*/,,' | sed -e 's;^lib\(boost_filesystem.*\)\.so.*$;\1;' -e 's;^lib\(boost_filesystem.*\)\.a*$;\1;' -e 's;^lib\(boost_filesystem.*\)\.dylib$;\1;'` ; do
                     ax_lib=${libextension}
                                    _BOOST_CHECK_LIB($ax_lib, [boost/filesystem/path.hpp], [boost::filesystem::path p;],
                                                     [BOOST_FILESYSTEM_LIB="-l$ax_lib"; AC_SUBST(BOOST_FILESYSTEM_LIB) link_filesystem="yes"; break],
                                                     [link_filesystem="no"])
dnl				    AC_CHECK_LIB($ax_lib, exit,
dnl                                 [BOOST_FILESYSTEM_LIB="-l$ax_lib"; AC_SUBST(BOOST_FILESYSTEM_LIB) link_filesystem="yes"; break],
dnl                                 [link_filesystem="no"])
  				done
                if test "x$link_program_options" != "xyes"; then
                for libextension in `ls $BOOSTLIBDIR/boost_filesystem*.{dll,a}* 2>/dev/null | sed 's,.*/,,' | sed -e 's;^\(boost_filesystem.*\)\.dll.*$;\1;' -e 's;^\(boost_filesystem.*\)\.a*$;\1;'` ; do
                     ax_lib=${libextension}
                                    _BOOST_CHECK_LIB($ax_lib, [boost/filesystem/path.hpp], [boost::filesystem::path p;],
                                                     [BOOST_FILESYSTEM_LIB="-l$ax_lib"; AC_SUBST(BOOST_FILESYSTEM_LIB) link_filesystem="yes"; break],
                                                     [link_filesystem="no"])
dnl				    AC_CHECK_LIB($ax_lib, exit,
dnl                                 [BOOST_FILESYSTEM_LIB="-l$ax_lib"; AC_SUBST(BOOST_FILESYSTEM_LIB) link_filesystem="yes"; break],
dnl                                 [link_filesystem="no"])
  				done
	            fi
            else
               for ax_lib in $ax_boost_user_filesystem_lib boost_filesystem-$ax_boost_user_filesystem_lib; do
                                    _BOOST_CHECK_LIB($ax_lib, [boost/filesystem/path.hpp], [boost::filesystem::path p;],
                                                     [BOOST_FILESYSTEM_LIB="-l$ax_lib"; AC_SUBST(BOOST_FILESYSTEM_LIB) link_filesystem="yes"; break],
                                                     [link_filesystem="no"])
dnl				      AC_CHECK_LIB($ax_lib, exit,
dnl                                   [BOOST_FILESYSTEM_LIB="-l$ax_lib"; AC_SUBST(BOOST_FILESYSTEM_LIB) link_filesystem="yes"; break],
dnl                                   [link_filesystem="no"])
                  done

            fi
			if test "x$link_filesystem" != "xyes"; then
				AC_MSG_ERROR(Could not link against $ax_lib !)
			fi
		fi

		CPPFLAGS="$CPPFLAGS_SAVED"
    		LDFLAGS="$LDFLAGS_SAVED"
		LIBS="$LIBS_SAVED"
	fi
])

AC_DEFUN([AX_BOOST_SERIALIZATION],
[
	AC_ARG_WITH([boost-serialization],
	AS_HELP_STRING([--with-boost-serialization@<:@=special-lib@:>@],
                   [use the Serialization library from boost - it is possible to specify a certain library for the linker
                        e.g. --with-boost-serialization=boost_serialization-gcc-mt ]),
        [
        if test "$withval" = "no"; then
			want_boost="no"
        elif test "$withval" = "yes"; then
            want_boost="yes"
            ax_boost_user_serialization_lib=""
        else
		    want_boost="yes"
        	ax_boost_user_serialization_lib="$withval"
		fi
        ],
        [want_boost="yes"]
	)

	if test "x$want_boost" = "xyes"; then
        AC_REQUIRE([AC_PROG_CC])
		CPPFLAGS_SAVED="$CPPFLAGS"
		CPPFLAGS="$CPPFLAGS $BOOST_CPPFLAGS"
		export CPPFLAGS

		LDFLAGS_SAVED="$LDFLAGS"
		LDFLAGS="$LDFLAGS $BOOST_LDFLAGS -lboost_serialization"
		export LDFLAGS

		LIBS_SAVED=$LIBS
		LIBS="$LIBS $BOOST_SYSTEM_LIB"
		export LIBS

        AC_CACHE_CHECK(whether the Boost::Serialization library is available,
					   ax_cv_boost_serialization,
        [AC_LANG_PUSH([C++])
         AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[@%:@include <boost/archive/text_oarchive.hpp>
	                                     @%:@include <sstream>]],
                                   [[
				   std::ostringstream out;
                                   boost::archive::text_oarchive oa(out);
                                   return 0;]])],
            				       ax_cv_boost_serialization=yes, ax_cv_boost_serialization=no)
         AC_LANG_POP([C++])
		])
		if test "x$ax_cv_boost_serialization" = "xyes"; then
			AC_DEFINE(HAVE_BOOST_SERIALIZATION,1,[define if the Boost::Serialization library is available])
            BOOSTLIBDIR=`echo $BOOST_LDFLAGS | sed -e 's/@<:@^\/@:>@*//'`
            if test "x$ax_boost_user_serialization_lib" = "x"; then
                for libextension in `ls $BOOSTLIBDIR/libboost_serialization*.{so,dylib,a}* 2>/dev/null | sed 's,.*/,,' | sed -e 's;^lib\(boost_serialization.*\)\.so.*$;\1;' -e 's;^lib\(boost_serialization.*\)\.a*$;\1;' -e 's;^lib\(boost_serialization.*\)\.dylib$;\1;'` ; do
                     ax_lib=${libextension}
				    AC_CHECK_LIB($ax_lib, main,
                                 [BOOST_SERIALIZATION_LIB="-l$ax_lib"; AC_SUBST(BOOST_SERIALIZATION_LIB) link_serialization="yes"; break],
                                 [link_serialization="no"])
  				done
                if test "x$link_program_options" != "xyes"; then
                for libextension in `ls $BOOSTLIBDIR/boost_serialization*.{dll,a}* 2>/dev/null | sed 's,.*/,,' | sed -e 's;^\(boost_serialization.*\)\.dll.*$;\1;' -e 's;^\(boost_serialization.*\)\.a*$;\1;'` ; do
                     ax_lib=${libextension}
				    AC_CHECK_LIB($ax_lib, main,
                                 [BOOST_SERIALIZATION_LIB="-l$ax_lib"; AC_SUBST(BOOST_SERIALIZATION_LIB) link_serialization="yes"; break],
                                 [link_serialization="no"])
  				done
	            fi
            else
               for ax_lib in $ax_boost_user_serialization_lib boost_serialization-$ax_boost_user_serialization_lib; do
				      AC_CHECK_LIB($ax_lib, exit,
                                   [BOOST_SERIALIZATION_LIB="-l$ax_lib"; AC_SUBST(BOOST_SERIALIZATION_LIB) link_serialization="yes"; break],
                                   [link_serialization="no"])
                  done

            fi
			if test "x$link_serialization" != "xyes"; then
				AC_MSG_ERROR(Could not link against $ax_lib !)
			fi
		fi

		CPPFLAGS="$CPPFLAGS_SAVED"
    		LDFLAGS="$LDFLAGS_SAVED"
		LIBS="$LIBS_SAVED"
	fi
])

AC_DEFUN([AX_BOOST_REGEX],
[
	AC_ARG_WITH([boost-regex],
	AS_HELP_STRING([--with-boost-regex@<:@=special-lib@:>@],
                   [use the Regex library from boost - it is possible to specify a certain library for the linker
                        e.g. --with-boost-regex=boost_regex-gcc-mt ]),
        [
        if test "$withval" = "no"; then
			want_boost="no"
        elif test "$withval" = "yes"; then
            want_boost="yes"
            ax_boost_user_regex_lib=""
        else
		    want_boost="yes"
        	ax_boost_user_regex_lib="$withval"
		fi
        ],
        [want_boost="yes"]
	)

	if test "x$want_boost" = "xyes"; then
        AC_REQUIRE([AC_PROG_CC])
		CPPFLAGS_SAVED="$CPPFLAGS"
		CPPFLAGS="$CPPFLAGS $BOOST_CPPFLAGS"
		export CPPFLAGS

		LDFLAGS_SAVED="$LDFLAGS"
		LDFLAGS="$LDFLAGS $BOOST_LDFLAGS" 
		export LDFLAGS

		LIBS_SAVED=$LIBS
		LIBS="$LIBS $BOOST_SYSTEM_LIB"
		export LIBS

        AC_CACHE_CHECK(whether the Boost::Regex library is available,
					   ax_cv_boost_regex,
        [AC_LANG_PUSH([C++])
         AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[@%:@include <boost/regex.hpp>
	                                     ]],
                                   [[
				   boost::regex r();
                                   return 0;]])],
            				       ax_cv_boost_regex=yes, ax_cv_boost_regex=no)
         AC_LANG_POP([C++])
		])
		if test "x$ax_cv_boost_regex" = "xyes"; then
			AC_DEFINE(HAVE_BOOST_REGEX,1,[define if the Boost::Regex library is available])
            BOOSTLIBDIR=`echo $BOOST_LDFLAGS | sed -e 's/@<:@^\/@:>@*//'`
            if test "x$ax_boost_user_regex_lib" = "x"; then
                for libextension in `ls $BOOSTLIBDIR/libboost_regex*.{so,dylib,a}* 2>/dev/null | sed 's,.*/,,' | sed -e 's;^lib\(boost_regex.*\)\.so.*$;\1;' -e 's;^lib\(boost_regex.*\)\.a*$;\1;' -e 's;^lib\(boost_regex.*\)\.dylib$;\1;'` ; do
                     ax_lib=${libextension}
				    dnl AC_CHECK_LIB($ax_lib, main,
                                 _BOOST_CHECK_LIB($ax_lib, [boost/regex.hpp], [boost::regex exp("*"); boost::regex_match("foo", exp);],
                                 [BOOST_REGEX_LIB="-l$ax_lib"; AC_SUBST(BOOST_REGEX_LIB) link_regex="yes"; break],
                                 [link_regex="no"])
  				done
                if test "x$link_program_options" != "xyes"; then
                for libextension in `ls $BOOSTLIBDIR/boost_regex*.{dll,a}* 2>/dev/null | sed 's,.*/,,' | sed -e 's;^\(boost_regex.*\)\.dll.*$;\1;' -e 's;^\(boost_regex.*\)\.a*$;\1;'` ; do
                     ax_lib=${libextension}
				 dnl   AC_CHECK_LIB($ax_lib, main,
                                 _BOOST_CHECK_LIB($ax_lib, [boost/regex.hpp], [boost::regex exp("*"); boost::regex_match("foo", exp);],
                                 [BOOST_REGEX_LIB="-l$ax_lib"; AC_SUBST(BOOST_REGEX_LIB) link_regex="yes"; break],
                                 [link_regex="no"])
  				done
	            fi
            else
                for ax_lib in $ax_boost_user_regex_lib boost_regex-$ax_boost_user_regex_lib; do
				   dnl   AC_CHECK_LIB($ax_lib, exit,
                                   _BOOST_CHECK_LIB($ax_lib, [boost/regex.hpp], [boost::regex exp("*"); boost::regex_match("foo", exp);],
                                   [BOOST_REGEX_LIB="-l$ax_lib"; AC_SUBST(BOOST_REGEX_LIB) link_regex="yes"; break],
                                   [link_regex="no"])
                  done

            fi
			if test "x$link_regex" != "xyes"; then
				AC_MSG_ERROR(Could not link against $ax_lib !)
			fi
		fi

		CPPFLAGS="$CPPFLAGS_SAVED"
    		LDFLAGS="$LDFLAGS_SAVED"
		LIBS="$LIBS_SAVED"
	fi
])

# ===========================================================================
#      http://www.gnu.org/software/autoconf-archive/ax_boost_system.html
# ===========================================================================
#
# SYNOPSIS
#
#   AX_BOOST_SYSTEM
#
# DESCRIPTION
#
#   Test for System library from the Boost C++ libraries. The macro requires
#   a preceding call to AX_BOOST_BASE. Further documentation is available at
#   <http://randspringer.de/boost/index.html>.
#
#   This macro calls:
#
#     AC_SUBST(BOOST_SYSTEM_LIB)
#
#   And sets:
#
#     HAVE_BOOST_SYSTEM
#
# LICENSE
#
#   Copyright (c) 2008 Thomas Porschberg <thomas@randspringer.de>
#   Copyright (c) 2008 Michael Tindal
#   Copyright (c) 2008 Daniel Casimiro <dan.casimiro@gmail.com>
#
#   Copying and distribution of this file, with or without modification, are
#   permitted in any medium without royalty provided the copyright notice
#   and this notice are preserved. This file is offered as-is, without any
#   warranty.


AC_DEFUN([AX_BOOST_SYSTEM],
[
	AC_ARG_WITH([boost-system],
	AS_HELP_STRING([--with-boost-system@<:@=special-lib@:>@],
                   [use the System library from boost - it is possible to specify a certain library for the linker
                        e.g. --with-boost-system=boost_system-gcc-mt ]),
        [
        if test "$withval" = "no"; then
			want_boost="no"
        elif test "$withval" = "yes"; then
            want_boost="yes"
            ax_boost_user_system_lib=""
        else
		    want_boost="yes"
		ax_boost_user_system_lib="$withval"
		fi
        ],
        [want_boost="yes"]
	)

	if test "x$want_boost" = "xyes"; then
        AC_REQUIRE([AC_PROG_CC])
        AC_REQUIRE([AC_CANONICAL_BUILD])
		CPPFLAGS_SAVED="$CPPFLAGS"
		CPPFLAGS="$CPPFLAGS $BOOST_CPPFLAGS"
		export CPPFLAGS

		LDFLAGS_SAVED="$LDFLAGS"
		LDFLAGS="$LDFLAGS $BOOST_LDFLAGS"
		export LDFLAGS

        AC_CACHE_CHECK(whether the Boost::System library is available,
					   ax_cv_boost_system,
        [AC_LANG_PUSH([C++])
			 CXXFLAGS_SAVE=$CXXFLAGS

			 AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[@%:@include <boost/system/error_code.hpp>]],
                                   [[boost::system::system_category]])],
                   ax_cv_boost_system=yes, ax_cv_boost_system=no)
			 CXXFLAGS=$CXXFLAGS_SAVE
             AC_LANG_POP([C++])
		])
		if test "x$ax_cv_boost_system" = "xyes"; then
			AC_SUBST(BOOST_CPPFLAGS)

			AC_DEFINE(HAVE_BOOST_SYSTEM,,[define if the Boost::System library is available])
            BOOSTLIBDIR=`echo $BOOST_LDFLAGS | sed -e 's/@<:@^\/@:>@*//'`

			LDFLAGS_SAVE=$LDFLAGS
            if test "x$ax_boost_user_system_lib" = "x"; then
                for libextension in `ls -r $BOOSTLIBDIR/libboost_system* 2>/dev/null | sed 's,.*/lib,,' | sed 's,\..*,,'` ; do
                     ax_lib=${libextension}
				    AC_CHECK_LIB($ax_lib, exit,
                                 [BOOST_SYSTEM_LIB="-l$ax_lib"; AC_SUBST(BOOST_SYSTEM_LIB) link_system="yes"; break],
                                 [link_system="no"])
				done
                if test "x$link_system" != "xyes"; then
                for libextension in `ls -r $BOOSTLIBDIR/boost_system* 2>/dev/null | sed 's,.*/,,' | sed -e 's,\..*,,'` ; do
                     ax_lib=${libextension}
				    AC_CHECK_LIB($ax_lib, exit,
                                 [BOOST_SYSTEM_LIB="-l$ax_lib"; AC_SUBST(BOOST_SYSTEM_LIB) link_system="yes"; break],
                                 [link_system="no"])
				done
                fi

            else
               for ax_lib in $ax_boost_user_system_lib boost_system-$ax_boost_user_system_lib; do
				      AC_CHECK_LIB($ax_lib, exit,
                                   [BOOST_SYSTEM_LIB="-l$ax_lib"; AC_SUBST(BOOST_SYSTEM_LIB) link_system="yes"; break],
                                   [link_system="no"])
                  done

            fi
            if test "x$ax_lib" = "x"; then
                AC_MSG_ERROR(Could not find a version of the library!)
            fi
			if test "x$link_system" = "xno"; then
				AC_MSG_ERROR(Could not link against $ax_lib !)
			fi
		fi

		CPPFLAGS="$CPPFLAGS_SAVED"
	LDFLAGS="$LDFLAGS_SAVED"
	fi
])

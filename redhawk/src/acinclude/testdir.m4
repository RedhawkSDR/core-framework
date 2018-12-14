build_tests="yes"
AC_ARG_WITH(tests,
        [AC_HELP_STRING([--without-tests],
                        [disables building of testing directory]) ],
        [ test "$withval" = "no" && build_tests="no" || build_tests="yes" ],
        [ build_tests="yes" ]
        )
AM_CONDITIONAL([BUILD_TESTS], [test "x$build_tests" = xyes] )
AC_SUBST(build_tests)


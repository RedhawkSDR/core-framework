dnl 
dnl AC_PROG_OMNIIDL
dnl
dnl Description
dnl 
dnl  Locates the omniidl program (part of omniORB
dnl  [http://omniorb.sourceforge.net]). Sets the output variable `IDL'.
dnl 
dnl Copyright (C) 2003, Alex Tingle <alex.autoconf@firetree.net>
dnl 
dnl License:
dnl GNU General Public License
dnl [http://www.gnu.org/software/ac-archive/htmldoc/COPYING.html]
dnl with this special exception
dnl [http://www.gnu.org/software/ac-archive/htmldoc/COPYING-Exception.html]. 
dnl 

AC_DEFUN([AC_PROG_OMNIIDL],[
  AC_CACHE_CHECK([for omniidl],IDL,[
    IDL=no

    # First, try and find omniidl inside the $OMNIORBBASE/bin directory
    if test "x$OMNIORBBASE" != x; then
      IDL="not found in $OMNIORBBASE"
      for ac_idlfile in `find $OMNIORBBASE/bin -name omniidl`
      do
        if test -x "$ac_idlfile"; then
          IDL="$ac_idlfile"
          break;
        fi
      done
    fi

    if test "x$IDL" = xno; then
      if test "x$prefix" != x && test "$prefix" != "NONE" && test -d "$prefix/bin"
      then
        ac_omniidl_path="$prefix/bin:$PATH"
      else
        ac_omniidl_path="$PATH"
      fi
      AC_PATH_PROG_QUIET(IDL,omniidl,no,[$ac_omniidl_path])
    fi

  ])
    if test "x$IDL" != xno; then
      # Check with which version of omniORB `omniidl' is compatible.
      echo "interface ConfTest { void m(); };" > conftest.idl
      if AC_TRY_COMMAND([$IDL -bcxx -Wbh=.hh conftest.idl]); then
        ac_corba_omniidl_orb=unknown
        grep 'omniORB3/CORBA.h' conftest.hh >/dev/null 2>/dev/null && ac_corba_omniidl_orb=omniORB3
        grep 'omniORB4/CORBA.h' conftest.hh >/dev/null 2>/dev/null && ac_corba_omniidl_orb=omniORB4
      else
        IDL=no
      fi
      rm -f conftest*
    fi
  # Stop if omniidl is not compatible with omniORB.
  if test "x$IDL" != xno && test "x$IDL" != "xnot found in $OMNIORBBASE" && \
     test "x$ac_corba_omniidl_orb" != "x$CORBA_ORB"
  then
    AC_MSG_ERROR([omniidl output is for $ac_corba_omniidl_orb. You can't use it with $CORBA_ORB.
  This can happen when you have more than one version of omniORB installed.
  You probably need to set PYTHONPATH to ensure that omniidl is using the
  correct python module (_omniidlmodule.so).])
  fi
])


dnl
dnl This is a verbatim copy of AC_PATH_PROG from acgeneral.m4, only with the
dnl AC_MSG_CHECKING & AC_MSG_RESULT removed.
dnl

dnl AC_PATH_PROG_QUIET(VARIABLE, PROG-TO-CHECK-FOR [, VALUE-IF-NOT-FOUND [, PATH]])
AC_DEFUN([AC_PATH_PROG_QUIET],
[# Extract the first word of "$2", so it can be a program name with args.
set dummy $2; ac_word=[$]2
dnl AC_MSG_CHECKING([for $ac_word])
AC_CACHE_VAL(ac_cv_path_$1,
[case "[$]$1" in
  /*)
  ac_cv_path_$1="[$]$1" # Let the user override the test with a path.
  ;;
  ?:/*)			 
  ac_cv_path_$1="[$]$1" # Let the user override the test with a dos path.
  ;;
  *)
  IFS="${IFS= 	}"; ac_save_ifs="$IFS"; IFS=":"
dnl $ac_dummy forces splitting on constant user-supplied paths.
dnl POSIX.2 word splitting is done only on the output of word expansions,
dnl not every word.  This closes a longstanding sh security hole.
  ac_dummy="ifelse([$4], , $PATH, [$4])"
  for ac_dir in $ac_dummy; do 
    test -z "$ac_dir" && ac_dir=.
    if test -f $ac_dir/$ac_word; then
      ac_cv_path_$1="$ac_dir/$ac_word"
      break
    fi
  done
  IFS="$ac_save_ifs"
dnl If no 3rd arg is given, leave the cache variable unset,
dnl so AC_PATH_PROGS will keep looking.
ifelse([$3], , , [  test -z "[$]ac_cv_path_$1" && ac_cv_path_$1="$3"
])dnl
  ;;
esac])dnl
$1="$ac_cv_path_$1"
dnl if test -n "[$]$1"; then
dnl   AC_MSG_RESULT([$]$1)
dnl else
dnl   AC_MSG_RESULT(no)
dnl fi
AC_SUBST($1)dnl
])


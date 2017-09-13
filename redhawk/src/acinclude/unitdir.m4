unitdir=/usr/lib/systemd/system
AC_ARG_WITH(unitdir,
        [AC_HELP_STRING([--with-unitdir@<:@=unit-dir-path@:>@],
                        [install systemd unit files @<:@Default: no, and path defaults to /usr/lib/systemd/system if not given@:>@])],
        [ case "${withval}" in 
               no)
                  install_systemdunits=0
               ;;
              yes)
                  install_systemdunits=1
               ;;
              *)
                 install_systemdunits=1                   
                 unitdir=${withval}
               ;;
           esac ],
        [use_systemd=0]
        )
AM_CONDITIONAL([INSTALL_SYSTEMDUNITS], [test "x$install_systemdunits" = x1])
AC_SUBST(unitdir)

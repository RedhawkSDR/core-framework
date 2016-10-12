dnl
dnl  AC_CORBA_OMNI_PLATFORM
dnl
dnl Description
dnl 
dnl  Autodetects the platform and sets necessary omniORB variables.
dnl  defines: <platform> <processor> __OSVERSION__
dnl  variables: PLATFORM_NAME, PLATFORM_DEFINITION,
dnl             OSVERSION, PROCESSOR_NAME, PROCESSOR_DEFINITION
dnl
dnl  Based upon the autoconf macros from OmniORB4
dnl  [http://omniorb.sourceforge.net].
dnl
dnl Copyright (C) 2003, Alex Tingle <alex.autoconf@firetree.net>
dnl 
dnl License:
dnl GNU General Public License
dnl [http://www.gnu.org/software/ac-archive/htmldoc/COPYING.html]
dnl with this special exception
dnl [http://www.gnu.org/software/ac-archive/htmldoc/COPYING-Exception.html]. 
dnl 

AC_DEFUN([AC_CORBA_OMNI_PLATFORM],[
  AC_REQUIRE([AC_CANONICAL_HOST])
  plat_name="Platform_Unknown"
  plat_def="__unknown_platform__"
  os_v="0"

  AH_TEMPLATE(__linux__,[for OmniORB on Linux, Cygwin])
  AH_TEMPLATE(__sunos__,[for OmniORB on SunOS (Solaris)])
  AH_TEMPLATE(__osf1__,[for OmniORB on OSF1 (Tru64)])
  AH_TEMPLATE(__hpux__,[for OmniORB on HPUX])
  AH_TEMPLATE(__nextstep__,[for OmniORB on NextStep])
  AH_TEMPLATE(__irix__,[for OmniORB on IRIX])
  AH_TEMPLATE(__aix__,[for OmniORB on AIX])
  AH_TEMPLATE(__darwin__,[for OmniORB on Darwin])
  AH_TEMPLATE(__freebsd__,[for OmniORB on FreeBSD])
  AH_TEMPLATE(__osr5__,[for OmniORB on OSR5])

  case "$host" in
    *-*-linux-*)   plat_name="Linux";    plat_def="__linux__";    os_v="2";;
    *-*-cygwin*)   plat_name="Cygwin";   plat_def="__linux__";    os_v="2";;
    *-*-solaris*)  plat_name="SunOS";    plat_def="__sunos__";    os_v="5";;
    *-*-osf3*)     plat_name="OSF1";     plat_def="__osf1__";     os_v="3";;
    *-*-osf4*)     plat_name="OSF1";     plat_def="__osf1__";     os_v="4";;
    *-*-osf5*)     plat_name="OSF1";     plat_def="__osf1__";     os_v="5";;
    *-*-hpux10*)   plat_name="HPUX";     plat_def="__hpux__";     os_v="10";;
    *-*-hpux11*)   plat_name="HPUX";     plat_def="__hpux__";     os_v="11";;
    *-*-nextstep*) plat_name="NextStep"; plat_def="__nextstep__"; os_v="3";;
    *-*-openstep*) plat_name="NextStep"; plat_def="__nextstep__"; os_v="3";;
    *-*-irix*)     plat_name="IRIX";     plat_def="__irix__";     os_v="6";;
    *-*-aix*)      plat_name="AIX";      plat_def="__aix__";      os_v="4";;
    *-*-darwin*)   plat_name="Darwin";   plat_def="__darwin__";   os_v="1";;
    *-*-freebsd3*) plat_name="FreeBSD";  plat_def="__freebsd__";  os_v="3";;
    *-*-freebsd4*) plat_name="FreeBSD";  plat_def="__freebsd__";  os_v="4";;
    *-*-freebsd5*) plat_name="FreeBSD";  plat_def="__freebsd__";  os_v="5";;
    *-*-sco*)      plat_name="OSR5";     plat_def="__osr5__";     os_v="5";;
  esac

  AC_SUBST(PLATFORM_NAME, $plat_name)
  AC_SUBST(PLATFORM_DEFINITION, $plat_def)
  AC_SUBST(OSVERSION, $os_v)
  AC_DEFINE_UNQUOTED($plat_def)
  AC_DEFINE_UNQUOTED(__OSVERSION__, $os_v,[for omniORB])

  proc_name="UnknownProcessor"
  proc_def="__processor_unknown__"

  AH_TEMPLATE(__x86__,[for OmniORB on x86Processor])
  AH_TEMPLATE(__sparc__,[for OmniORB on SparcProcessor])
  AH_TEMPLATE(__alpha__,[for OmniORB on AlphaProcessor])
  AH_TEMPLATE(__m68k__,[for OmniORB on m68kProcessor])
  AH_TEMPLATE(__mips__,[for OmniORB on IndigoProcessor])
  AH_TEMPLATE(__arm__,[for OmniORB on ArmProcessor])
  AH_TEMPLATE(__s390__,[for OmniORB on s390Processor])
  AH_TEMPLATE(__ia64__,[for OmniORB on ia64Processor])
  AH_TEMPLATE(__hppa__,[for OmniORB on HppaProcessor])
  AH_TEMPLATE(__powerpc__,[for OmniORB on PowerPCProcessor])

  case "$host" in
    i?86-*)   proc_name="x86Processor";     proc_def="__x86__";;
    sparc-*)  proc_name="SparcProcessor";   proc_def="__sparc__";;
    alpha*)   proc_name="AlphaProcessor";   proc_def="__alpha__";;
    m68k-*)   proc_name="m68kProcessor";    proc_def="__m68k__";;
    mips*)    proc_name="IndigoProcessor";  proc_def="__mips__";;
    arm-*)    proc_name="ArmProcessor";     proc_def="__arm__";;
    s390-*)   proc_name="s390Processor";    proc_def="__s390__";;
    ia64-*)   proc_name="ia64Processor";    proc_def="__ia64__";;
    hppa*)    proc_name="HppaProcessor";    proc_def="__hppa__";;
    powerpc*) proc_name="PowerPCProcessor"; proc_def="__powerpc__";;
  esac

  AC_SUBST(PROCESSOR_NAME, $proc_name)
  AC_SUBST(PROCESSOR_DEFINITION, $proc_def)
  AC_DEFINE_UNQUOTED($proc_def)
])

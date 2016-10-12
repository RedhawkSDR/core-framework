#
# This file is protected by Copyright. Please refer to the COPYRIGHT file
# distributed with this source distribution.
#
# This file is part of REDHAWK core.
#
# REDHAWK core is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
#
# REDHAWK core is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see http://www.gnu.org/licenses/.
#

%{!?_ossiehome:  %define _ossiehome  /usr/local/redhawk/core}
%{!?_sdrroot:    %define _sdrroot    /var/redhawk/sdr}
%define _prefix %{_ossiehome}
Prefix:         %{_ossiehome}
Prefix:         %{_sdrroot}
Prefix:         %{_sysconfdir}

Name:           redhawk
Version:        2.0.3
Release:        2%{?dist}
Summary:        REDHAWK is a Software Defined Radio framework

Group:          Applications/Engineering
License:        LGPLv3+
URL:            http://redhawksdr.org/
Source:         %{name}-%{version}.tar.gz
Vendor:         REDHAWK

%define __arch_install_post %{nil}

Requires:       util-linux-ng

%if 0%{?rhel} >= 7 || 0%{?fedora} >= 17
Requires:       java >= 1.7
%else
Requires:       java7 >= 1.7
%endif

Requires:       python
Requires:       numpy
Requires:       python-omniORB >= 3.0
Requires:       omniORB-devel >= 4.1.0
Requires:       binutils
BuildRequires:  libuuid-devel
BuildRequires:  boost-devel >= 1.41
BuildRequires:  autoconf automake libtool
BuildRequires:  expat-devel

%if 0%{?rhel} >= 7 || 0%{?fedora} >= 17
BuildRequires:  java-devel >= 1.7
%else
BuildRequires:  java7-devel >= 1.7
%endif

BuildRequires:  python-devel >= 2.4
BuildRequires:  log4cxx-devel >= 0.10
BuildRequires:  omniORB-devel >= 4.1.0
BuildRequires:  omniORBpy-devel >= 3.0
BuildRequires:  libomniEvents2-devel
BuildRequires:  xsd >= 3.3.0

%description
REDHAWK is a Software Defined Radio framework.
 * Commit: __REVISION__
 * Source Date/Time: __DATETIME__

%package qt-tools
Summary:        PyQt Tools
Group:          Applications/Engineering
Requires:       %{name} = %{version}-%{release}
Requires:       PyQt4

%description qt-tools
PyQt-based applications (qtbrowse and rhlauncher)

%package sdrroot-dom-mgr
Summary:        Domain Manager
Group:          Applications/Engineering
Requires:       %{name} = %{version}-%{release}
Provides:       DomainManager = %{version}-%{release}

%description sdrroot-dom-mgr
The Domain Manager software package

%package sdrroot-dom-profile
Summary:        Basic domain manager profile
Group:          Applications/Engineering
Requires:       %{name}-sdrroot-dom-mgr = %{version}-%{release}

%description sdrroot-dom-profile
A generic domain profile and domain profile template

%package sdrroot-dev-mgr
Summary:        Device Manager
Group:          Applications/Engineering
Requires:       %{name} = %{version}-%{release}
Provides:       DeviceManager = %{version}-%{release}

%description sdrroot-dev-mgr
The Device Manager software package

%package devel
Summary:        The REDHAWK development package
Group:          Applications/Engineering

# REDHAWK
Requires:       redhawk = %{version}-%{release}

# Base dependencies
Requires:       libuuid-devel
Requires:       boost-devel >= 1.41
Requires:       autoconf automake libtool
Requires:       log4cxx-devel >= 0.10

# omniORB / omniORBpy
Requires:       omniORB-devel >= 4.1.0
Requires:       omniORBpy-devel >= 3.0

# Languages
Requires:       gcc-c++
Requires:       python-devel >= 2.4

%if 0%{?rhel} >= 7 || 0%{?fedora} >= 17
Requires:       java-devel >= 1.7
%else
Requires:       java7-devel >= 1.7
%endif

%description devel
This package ensures that all requirements for REDHAWK development are installed. It also provides a useful development utilities.


%prep
%setup -q


%build
# build the core framework
cd src
./reconf
%configure --with-sdr=%{_sdrroot} --with-pyscheme=home
make %{?_smp_mflags}


%install
rm -rf --preserve-root $RPM_BUILD_ROOT

# install ossie framework
cd src
make install DESTDIR=$RPM_BUILD_ROOT


%clean
rm -rf --preserve-root $RPM_BUILD_ROOT


%pre
# -r is system account, -f is force (ignore already exists)
groupadd -r -f redhawk
if ! id redhawk &> /dev/null; then
  # -M is don't create home dir, -r is system account, -s is shell
  # -c is comment, -n is don't create group, -g is group name/id
  /usr/sbin/useradd -M -r -s /sbin/nologin \
    -c "REDHAWK System Account" -n -g redhawk redhawk > /dev/null
fi


%files
%defattr(-,root,root,-)
%{_bindir}
%exclude %{_bindir}/prf2py.py
%exclude %{_bindir}/py2prf
%exclude %{_bindir}/qtbrowse
%exclude %{_bindir}/rhlauncher
%dir %{_includedir}
%dir %{_prefix}/lib
%ifarch x86_64
%dir %{_prefix}/lib64
%endif
%{_prefix}/lib/CFInterfaces.jar
%{_prefix}/lib/apache-commons-lang-2.4.jar
%{_prefix}/lib/log4j-1.2.15.jar
%{_prefix}/lib/ossie.jar
%{_prefix}/lib/python
%exclude %{_prefix}/lib/python/ossie/apps/qtbrowse
%exclude %{_prefix}/lib/python/ossie/apps/rhlauncher
%{_libdir}/libomnijni.so.*
%{_libdir}/libossiecf.so.*
%{_libdir}/libossiecfjni.so.*
%{_libdir}/libossieidl.so.*
%{_libdir}/libossieparser.so.*
%{_libdir}/libossielogcfg.so.*
%dir %{_libdir}/pkgconfig
%{_datadir}
%{_sysconfdir}/profile.d/redhawk.csh
%{_sysconfdir}/profile.d/redhawk.sh
%{_sysconfdir}/ld.so.conf.d/redhawk.conf

%files qt-tools
%defattr(-,root,root,-)
%{_bindir}/qtbrowse
%{_bindir}/rhlauncher
%{_prefix}/lib/python/ossie/apps/qtbrowse
%{_prefix}/lib/python/ossie/apps/rhlauncher

%files sdrroot-dom-mgr
%defattr(664,redhawk,redhawk)
%attr(2775,redhawk,redhawk) %dir %{_sdrroot}
%attr(2775,redhawk,redhawk) %dir %{_sdrroot}/dom
%attr(2775,redhawk,redhawk) %dir %{_sdrroot}/dom/components
%attr(2775,redhawk,redhawk) %dir %{_sdrroot}/dom/deps
%attr(2775,redhawk,redhawk) %dir %{_sdrroot}/dom/domain
%attr(2775,redhawk,redhawk) %dir %{_sdrroot}/dom/mgr
%attr(775,redhawk,redhawk) %{_sdrroot}/dom/mgr/DomainManager
%{_sdrroot}/dom/mgr/*.xml
%attr(2775,redhawk,redhawk) %dir %{_sdrroot}/dom/waveforms
%attr(644,root,root) %{_sysconfdir}/profile.d/redhawk-sdrroot.csh
%attr(644,root,root) %{_sysconfdir}/profile.d/redhawk-sdrroot.sh

%files sdrroot-dom-profile
%defattr(664,redhawk,redhawk)
%config %{_sdrroot}/dom/domain/DomainManager.dmd.xml
%{_sdrroot}/dom/domain/DomainManager.dmd.xml.template

%files sdrroot-dev-mgr
%defattr(664,redhawk,redhawk)
%attr(2775,redhawk,redhawk) %dir %{_sdrroot}
%attr(2775,redhawk,redhawk) %dir %{_sdrroot}/dev
%attr(2775,redhawk,redhawk) %dir %{_sdrroot}/dev/devices
%attr(2775,redhawk,redhawk) %dir %{_sdrroot}/dev/mgr
%attr(2775,redhawk,redhawk) %dir %{_sdrroot}/dev/nodes
%attr(775,redhawk,redhawk) %{_sdrroot}/dev/mgr/DeviceManager
%{_sdrroot}/dev/mgr/DeviceManager.*
%attr(644,root,root) %{_sysconfdir}/profile.d/redhawk-sdrroot.csh
%attr(644,root,root) %{_sysconfdir}/profile.d/redhawk-sdrroot.sh

%files devel
%defattr(-,root,root,-)
%{_bindir}/prf2py.py
%{_bindir}/py2prf
%{_includedir}/ossie
%{_libdir}/libomnijni.*a
%{_libdir}/libomnijni.so
%{_libdir}/libossiecf.*a
%{_libdir}/libossiecf.so
%{_libdir}/libossiecfjni.*a
%{_libdir}/libossiecfjni.so
%{_libdir}/libossieidl.so
%{_libdir}/libossieidl.*a
%{_libdir}/libossieparser.*a
%{_libdir}/libossieparser.so
%{_libdir}/libossielogcfg.*a
%{_libdir}/libossielogcfg.so
%{_libdir}/pkgconfig/ossie.pc
%{_sysconfdir}/bash_completion.d/nodeBooter


%post
/sbin/ldconfig

%postun
/sbin/ldconfig


%changelog
* Wed Sep 9 2015 - 2.0.0-2
- Add qt-tools package
- Remove el5 support

* Wed Sep 15 2014 - 1.11.0-1
- Update for dependency on java7

* Wed May 21 2014 - 1.10.0-7
- Move a dependency that was on the wrong package

* Fri Apr 11 2014 - 1.10.0-5
- Improve OS version detection for RHEL/CentOS/Fedora
- Don't constrain boost to exact version
- Exclude qtbrowse on el5, return it to base package
- Clarify useradd/groupadd
- Add missing package requirement
- Switch to omniORBpy packaging that is compatible with Fedora

* Thu Aug 15 2013 - 1.9.0-1
- Re-work lots of dependencies
- Package for REDHAWK development
- Minor fixes for docs, licensing
- Explicitly require Java for build

* Tue Mar 12 2013 - 1.8.3-4
- Update licensing information
- Add URL for website
- Change group to a standard one, per Fedora
- Remove standard build dependencies

* Mon Sep 19 2011 - 1.7.2-2
- Further split RPMs to allow more granularity in installations

* Tue Jun 07 2011 - 1.7.0-0
- Split sdrroot into -dev and dom
- Attempt to fully capture Requires and BuildRequires
- Stop packaging components into SDRROOT

* Tue Jan 11 2011 - 1.6.0-0
- Initial spec file for redhawk and redhawk-sdrroot.


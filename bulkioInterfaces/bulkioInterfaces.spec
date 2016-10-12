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

# By default, the RPM will install to the standard REDHAWK OSSIE root location (/usr/local/redhawk/core)
# You can override this at install time using --prefix /usr/local/redhawk/core when invoking rpm (preferred)
%{!?_ossiehome: %global _ossiehome /usr/local/redhawk/core}
%define _prefix %{_ossiehome}
Prefix:         %{_prefix}

# Assume Java support by default. Use "rpmbuild --without java" to disable
%bcond_without java

Name:           bulkioInterfaces
Version:        1.8.8
Release:        1%{?dist}
Summary:        The bulkio library for REDHAWK

Group:          Applications/Engineering
License:        LGPLv3+
URL:            http://redhawksdr.org/
Source:         %{name}-%{version}.tar.gz

BuildRoot: 	%{_tmppath}/%{name}-%{version}-%{release}-buildroot

Requires: 	redhawk >= 1.8
BuildRequires: 	redhawk >= 1.8
BuildRequires: 	autoconf automake libtool
BuildRequires: 	omniORB
BuildRequires: 	python omniORBpy omniORBpy-devel
BuildRequires: 	apache-log4cxx-devel
%if 0%{?rhel} >= 6 || 0%{?fedora} >= 12
BuildRequires: 	libuuid-devel
%else
BuildRequires: 	e2fsprogs-devel
%endif
%if %{with java}
Requires:       java >= 1.6
BuildRequires: 	java-devel >= 1.6
BuildRequires:  jpackage-utils
%endif


%description
Libraries and interface definitions for bulkio interfaces.
 * Commit: __REVISION__
 * Source Date/Time: __DATETIME__



%prep
%setup -q


%build
./reconf
%configure %{?_without_java: --disable-java}
make %{?_smp_mflags}


%install
rm -rf --preserve-root $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT


%clean
rm -rf --preserve-root $RPM_BUILD_ROOT


%files
%defattr(-,redhawk,redhawk)
%{_datadir}/idl/ossie/BULKIO
%{_includedir}/ossie/BULKIO
%{_libdir}/libbulkioInterfaces.*
%{_libdir}/pkgconfig/bulkioInterfaces.pc
%{_prefix}/lib/python/bulkio
%if 0%{?rhel} >= 6 || 0%{?fedora} >= 12
%{_prefix}/lib/python/bulkioInterfaces-%{version}-py%{python_version}.egg-info
%endif
%if %{with java}
%{_prefix}/lib/BULKIOInterfaces.jar
%{_prefix}/lib/BULKIOInterfaces.src.jar
%{_prefix}/%{_lib}/libbulkiojni.*
%endif


%post
/sbin/ldconfig

%postun
/sbin/ldconfig


%changelog
* Fri Mar 21 2014 1.8.7-1
- Improve OS version detection for RHEL/CentOS/Fedora

* Fri Mar 29 2013 1.8.4
- Re-work java use
- Remove unneeded defines
- Parallelize build

* Tue Mar 12 2013 1.8.3-4
- Update licensing information
- Add URL for website
- Change group to a standard one, per Fedora
- Quiet the file unpack


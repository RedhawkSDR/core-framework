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

Name:           burstioInterfaces
Version:        1.10.1
Release:        3%{?dist}
Summary:        BURSTIO interfaces for REDHAWK

Group:          Applications/Engineering
License:        LGPLv3+
URL:            http://redhawksdr.org/
Source:         %{name}-%{version}.tar.gz
Vendor:         REDHAWK

BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-buildroot

Requires:       redhawk >= 1.8
BuildRequires:  redhawk-devel >= 1.8

Requires:       bulkioInterfaces >= 1.8
BuildRequires:  bulkioInterfaces >= 1.8

%description
BURSTIO interfaces for REDHAWK
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
%defattr(-,root,root,-)
%{_datadir}/idl/redhawk/BURSTIO
%{_includedir}/redhawk/BURSTIO
%{_includedir}/redhawk/burstio
%{_prefix}/lib/python/redhawk/burstio
%{_prefix}/lib/python/redhawk/burstioInterfaces
%if 0%{?rhel} >= 6
%{_prefix}/lib/python/burstio-%{version}-py%{python_version}.egg-info
%{_prefix}/lib/python/burstioInterfaces-%{version}-py%{python_version}.egg-info
%endif
%if %{with java}
%{_prefix}/lib/BURSTIOInterfaces.jar
%{_prefix}/lib/burstio.jar
%{_libdir}/pkgconfig/burstio.pc
%{_libdir}/pkgconfig/burstioInterfaces.pc
%{_libdir}/libburstioInterfaces.so.*
%{_libdir}/libburstio.so.*
%{_libdir}/libburstiojni.so.*
%{_libdir}/libburstioInterfaces.so
%{_libdir}/libburstio.so
%{_libdir}/libburstiojni.so
%{_libdir}/libburstioInterfaces.*a
%{_libdir}/libburstio.*a
%{_libdir}/libburstiojni.*a
%endif



%post
/sbin/ldconfig

%postun
/sbin/ldconfig


%changelog
* Thu Feb 20 2014 1.10.0
- Initial commit

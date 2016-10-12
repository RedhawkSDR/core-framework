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
%{!?_ossiehome: %global _ossiehome /usr/local/redhawk/core}
%define _prefix %{_ossiehome}
Prefix: %{_prefix}

# Point install paths to locations within our target OSSIE root
%define _sysconfdir    %{_prefix}/etc
%define _localstatedir %{_prefix}/var
%define _mandir        %{_prefix}/man
%define _infodir       %{_prefix}/info

# Assume Java support by default. Use "rpmbuild --without java" to disable
%bcond_without java

Summary:        The frontend library for REDHAWK
Name:           frontendInterfaces
Version:        2.2.1
Release:        3%{?dist}
License:        LGPLv3+
Group:          REDHAWK/Interfaces
Source:         %{name}-%{version}.tar.gz
Vendor:         REDHAWK

BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-buildroot

Requires:       redhawk >= 1.10.0, redhawk < 1.11.0
Requires:       bulkioInterfaces >= 1.10.0, bulkioInterfaces < 1.11.0
BuildRequires:  redhawk-devel >= 1.10.0, redhawk-devel < 1.11.0
BuildRequires:  bulkioInterfaces >= 1.10.0, bulkioInterfaces < 1.11.0

%description
Libraries and interface definitions for frontend.
 * Commit: __REVISION__
 * Source Date/Time: __DATETIME__


%prep
%setup

%build
./reconf
%configure %{?_without_java: --disable-java}
make

%install
rm -rf --preserve-root $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT

# The Java and Python base class libraries are not yet fully implemented. We
# include the Python one (because it has important Sandbox helpers), but not
# the Java one yet.
%if %{with java}
rm -rf $RPM_BUILD_ROOT%{_prefix}/lib/frontend.jar
rm -rf $RPM_BUILD_ROOT%{_prefix}/lib/frontend.src.jar
%endif


%clean
rm -rf --preserve-root $RPM_BUILD_ROOT

%files
%defattr(-,root,root,-)
%{_datadir}/idl/redhawk/FRONTEND
%{_includedir}/redhawk/FRONTEND
%{_prefix}/lib/python/redhawk/frontendInterfaces
%{_prefix}/lib/python/frontend
%{_libdir}/libfrontendInterfaces.*
%{_libdir}/pkgconfig/frontendInterfaces.pc
%{_includedir}/frontend
%{_libdir}/libfrontend-*
%{_libdir}/pkgconfig/frontend.pc
%if 0%{?rhel} >= 6
%{_prefix}/lib/python/frontendInterfaces-%{version}-py%{python_version}.egg-info
%{_prefix}/lib/python/frontend-%{version}-py%{python_version}.egg-info
%endif
%if %{with java}
%{_prefix}/lib/FRONTENDInterfaces.jar
%{_prefix}/lib/FRONTENDInterfaces.src.jar
%endif

%post
/sbin/ldconfig

%postun
/sbin/ldconfig

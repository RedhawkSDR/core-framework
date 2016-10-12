#
# This file is protected by Copyright. Please refer to the COPYRIGHT file
# distributed with this source distribution.
#
# This file is part of REDHAWK bulkioInterfaces.
#
# REDHAWK bulkioInterfaces is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
#
# REDHAWK bulkioInterfaces is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see http://www.gnu.org/licenses/.
#
# By default, the RPM will install to the standard REDHAWK SDR root location (/var/redhawk/sdr)
# You can override this at install time using --prefix /new/sdr/root when invoking rpm (preferred method, if you must)
%{!?_sdrroot: %define _sdrroot /var/redhawk/sdr}
%define _prefix %{_sdrroot}
Prefix: %{_prefix}

# Point install paths to locations within our target SDR root
%define _sysconfdir    %{_prefix}/etc
%define _localstatedir %{_prefix}/var
%define _mandir        %{_prefix}/man
%define _infodir       %{_prefix}/info

Name: TestCPP
Summary: Component %{name}
Version: 1.0.0
Release: 1
License: None
Group: REDHAWK/Components
Source: %{name}-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-root

Requires: redhawk >= 1.8
BuildRequires: redhawk >= 1.8
BuildRequires: autoconf automake libtool

# Interface requirements
Requires: bulkioInterfaces
BuildRequires: bulkioInterfaces

# C++ requirements
Requires: libomniORB4.1
Requires: boost >= 1.41
Requires: apache-log4cxx >= 0.10
BuildRequires: boost-devel >= 1.41
BuildRequires: libomniORB4.1-devel
BuildRequires: apache-log4cxx-devel >= 0.10

# Java requirements
Requires: java
BuildRequires: jdk

# Python requirements
Requires: python omniORBpy
BuildRequires: libomniORBpy3-devel
BuildRequires: python-devel >= 2.3

%description
Component %{name}

%prep
%setup

%build
# Implementation cpp
pushd cpp
./reconf
%define _bindir %{_prefix}/dom/components/TestCPP/cpp
%configure
make
popd
# Implementation python
pushd python
./reconf
%define _bindir %{_prefix}/dom/components/TestCPP/python
%configure
make
popd
# Implementation java
pushd java
./reconf
%define _bindir %{_prefix}/dom/components/TestCPP/java
%configure
make
popd
# Implementation java_impl2
pushd 
./reconf
%define _bindir %{_prefix}/dom/components/TestCPP/
%configure
make
popd

%install
rm -rf $RPM_BUILD_ROOT
# Implementation cpp
pushd cpp
%define _bindir %{_prefix}/dom/components/TestCPP/cpp
make install DESTDIR=$RPM_BUILD_ROOT
popd
# Implementation python
pushd python
%define _bindir %{_prefix}/dom/components/TestCPP/python
make install DESTDIR=$RPM_BUILD_ROOT
popd
# Implementation java
pushd java
%define _bindir %{_prefix}/dom/components/TestCPP/java
make install DESTDIR=$RPM_BUILD_ROOT
popd
# Implementation java_impl2
pushd 
%define _bindir %{_prefix}/dom/components/TestCPP/
make install DESTDIR=$RPM_BUILD_ROOT
popd

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,redhawk,redhawk)
%dir %{_prefix}/dom/components/%{name}
%{_prefix}/dom/components/%{name}/TestCPP.scd.xml
%{_prefix}/dom/components/%{name}/TestCPP.prf.xml
%{_prefix}/dom/components/%{name}/TestCPP.spd.xml
%{_prefix}/dom/components/%{name}/cpp
%{_prefix}/dom/components/%{name}/python
%{_prefix}/dom/components/%{name}/java
%{_prefix}/dom/components/%{name}/


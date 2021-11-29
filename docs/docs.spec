# This file is protected by Copyright. Please refer to the COPYRIGHT file
# distributed with this source distribution.
#
# This file is part of REDHAWK.
#
# REDHAWK is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
#
# REDHAWK is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see http://www.gnu.org/licenses/.

Name:             redhawk-docs
Version:          3.0.0
Release:          1%{?dist}
Summary:          The Redhawk Manual

License:          LGPLv3+
Group:            Applications/Engineering
URL:              https://redhawksdr.org
Source:           %{name}-%{version}.tar.gz
Vendor:           REDHAWK

BuildArch:        noarch

BuildRequires:    make
BuildRequires:    pandoc
BuildRequires:    python3
BuildRequires:    redhawk >= 3.0.0

%{!?_ossiehome:  %global _ossiehome  /usr/local/redhawk/core}
%global _prefix %{_ossiehome}
Prefix: %{_ossiehome}

%description
REDHAWK Manual
 * Commit: __REVISION__
 * Source Date/Time: __DATETIME__

%prep
%setup

%build

%install
mkdir -p %{buildroot}%{_prefix}/docs
cp -dr css font img js md py Makefile README.md %{buildroot}%{_prefix}/docs/

%files
%{_prefix}/docs/css
%{_prefix}/docs/font
%{_prefix}/docs/img
%{_prefix}/docs/js
%{_prefix}/docs/md
%{_prefix}/docs/py
%{_prefix}/docs/Makefile
%{_prefix}/docs/README*

%post
cd %{_prefix}/docs && make
cd %{_prefix}/docs && make install


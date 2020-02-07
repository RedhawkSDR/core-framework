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
%{!?python_sitelib: %define python_sitelib %(%{__python} -c "from distutils.sysconfig import get_python_lib; print get_python_lib()")}
%{!?_ossiehome:  %define _ossiehome  /usr/local/redhawk/core}
%define _prefix %{_ossiehome}
Prefix:         %{_prefix}

Name:           redhawk-codegen
Version:        2.2.6
Release:        1%{?dist}
Summary:        Redhawk Code Generators

Group:          Applications/Engineering
License:        LGPLv3+
URL:            http://redhawksdr.org/
Source:         %{name}-%{version}.tar.gz
Vendor:         REDHAWK

BuildArch:      noarch
# BuildRoot required for el5
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-buildroot

Requires:       python
Requires:       redhawk = %{version}
%if 0%{?rhel} >= 7
Requires:       python-jinja2
BuildRequires:  python-jinja2
%else
Requires:       python-jinja2-26
BuildRequires:  python-jinja2-26
%endif

BuildRequires:  python-setuptools
BuildRequires:  python-devel >= 2.4

# Turn off the brp-python-bytecompile script; our setup.py does byte compilation
# (From https://fedoraproject.org/wiki/Packaging:Python#Bytecompiling_with_the_correct_python_version)
%global __os_install_post %(echo '%{__os_install_post}' | sed -e 's!/usr/lib[^[:space:]]*/brp-python-bytecompile[[:space:]].*$!!g')

%description
REDHAWK Code Generators
 * Commit: __REVISION__
 * Source Date/Time: __DATETIME__


%prep
%setup -q


%build
%{__python} setup.py build


%install
rm -rf $RPM_BUILD_ROOT
%{__python} setup.py install --skip-build -O1 --home=%{_prefix} --root=%{buildroot}
rm $RPM_BUILD_ROOT%{_prefix}/lib/python/redhawk/__init__.py*


%clean
rm -rf $RPM_BUILD_ROOT


%files
%defattr(-,root,root,-)
%{_bindir}/codegen_version
%{_bindir}/createBinaryComponent
%{_bindir}/createOctaveComponent
%{_bindir}/moveComponentNamespace
%{_bindir}/update_project
%{_bindir}/redhawk-codegen
%{_prefix}/lib/python/redhawk/codegen
%{_prefix}/lib/python/redhawk/packagegen
%if 0%{?rhel} >= 6
%{_prefix}/lib/python/redhawk_codegen-%{version}-py%{python_version}.egg-info
%endif

%changelog
* Wed Jun 28 2017 Ryan Bauman <rbauman@lgsinnovations.com> - 2.1.2-1
- Bump for 2.1.2-rc1

* Wed Jun 28 2017 Ryan Bauman <rbauman@lgsinnovations.com> - 2.1.1-2
- Bump for 2.1.1-rc2

* Thu May 21 2015 - 2.0.0-2
- Update python-jinja2 package for el7


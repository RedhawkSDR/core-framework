#{#
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
#}
#$ set name = component.name
#$ set dirname = component.name.replace('.','/')
# By default, the RPM will install to the standard REDHAWK SDR root location (/var/redhawk/sdr)
# You can override this at install time using --prefix /new/sdr/root when invoking rpm (preferred method, if you must)
%{!?_sdrroot: %global _sdrroot /var/redhawk/sdr}
%define _prefix %{_sdrroot}
Prefix:         %{_prefix}

# Point install paths to locations within our target SDR root
%define _sysconfdir    %{_prefix}/etc
%define _localstatedir %{_prefix}/var
%define _mandir        %{_prefix}/man
%define _infodir       %{_prefix}/info

#{$ block variableExtensions $}
#{$ endblock $}
Name:           {{name}}
Version:        {{component.version}}
Release:        1%{?dist}
Summary:        GPP Plugin %{name}

Group:          REDHAWK/{{component.type}}s
License:        None
Source0:        %{name}-%{version}.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires:  redhawk-devel >= {{versions.redhawk}}
Requires:       redhawk >= {{versions.redhawk}}
#{$ block requireExtensions $}
#{$ endblock $}

#{$ for impl in component.implementations $}
#{$ for softpkgdep in impl.softpkgdeps $}
BuildRequires:  {{softpkgdep.name}}-devel
Requires:       {{softpkgdep.name}}
#{$ endfor $}
#{$ endfor $}

#{$ if component.interfaces $}
# Interface requirements
BuildRequires:  {{component.interfaces|join(' ')}}
Requires:       {{component.interfaces|join(' ')}}

#{$ endif $}
#{$ if 'C++' not in component.languages and component.languages $}
BuildArch: noarch

#{$ endif $}
#{$ for impl in component.implementations if impl.requires or impl.buildrequires $}
# Implementation {{impl.id}}
#{$   if impl.requires $}
Requires: {{impl.requires|join(' ')}}
#{$   endif $}
#{$   if impl.buildrequires $}
BuildRequires: {{impl.buildrequires|join(' ')}}
#{$   endif $}
#{$ endfor $}

%description
GPP Plugin %{name}
 * Commit: __REVISION__
 * Source Date/Time: __DATETIME__


%prep
%setup -q


%build
./reconf
%define _bindir %{_prefix}/dev/devices/GPP/plugins/{{name}}
%configure
make %{?_smp_mflags}


%install
rm -rf $RPM_BUILD_ROOT
%define _bindir %{_prefix}/dev/devices/GPP/plugins/{{name}}
make install DESTDIR=$RPM_BUILD_ROOT


%clean
rm -rf $RPM_BUILD_ROOT


%files
%defattr(-,redhawk,redhawk,-)
%dir %{_sdrroot}/dev/devices/GPP/plugins/{{name}}
%{_prefix}/dev/devices/GPP/plugins/{{name}}/{{name}}

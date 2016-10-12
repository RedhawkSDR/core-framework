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
# By default, the RPM will install to the standard REDHAWK SDR root location (/var/redhawk/sdr)
# You can override this at install time using --prefix /new/sdr/root when invoking rpm (preferred method, if you must)
%{!?_sdrroot: %define _sdrroot /var/redhawk/sdr}
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
Summary:        {{component.type}} %{name}{{' '+component.title if component.title}}

Group:          REDHAWK/{{component.type}}s
License:        None
Source0:        %{name}-%{version}.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires:  redhawk-devel >= {{versions.redhawk}}
Requires:       redhawk >= {{versions.redhawk}}
#{$ block requireExtensions $}
#{$ endblock $}

#{$ if component.interfaces $}
# Interface requirements
BuildRequires:  {{component.interfaces|join(' ')}}
Requires:       {{component.interfaces|join(' ')}}

#{$ endif $}
#{$ if 'C++' not in component.languages and component.languages $}
BuildArch: noarch

#{$ endif $}
#{$ if 'Java' in component.languages and component.languages $}
# Java requirements
Requires: java >= 1.6
BuildRequires: java-devel >= 1.6

#{$ endif $}

%description
#{$ if component.description $}
{{component.description}}
#{$ else $}
{{component.type}} %{name}
#{$ endif $}


%prep
%setup -q


%build
#{$ block build $}
#{$ for impl in component.implementations $}
# Implementation {{impl.id}}
pushd {{impl.outputdir}}
./reconf
%define _bindir %{_prefix}/{{component.sdrpath}}/{{name}}/{{impl.outputdir}}
%configure
make %{?_smp_mflags}
popd
#{$ endfor $}
#{$ endblock  $}


%install
#{$ block install $}
rm -rf $RPM_BUILD_ROOT
#{$ for impl in component.implementations $}
# Implementation {{impl.id}}
pushd {{impl.outputdir}}
%define _bindir %{_prefix}/{{component.sdrpath}}/{{name}}/{{impl.outputdir}}
make install DESTDIR=$RPM_BUILD_ROOT
popd
#{$ endfor $}
#{$ endblock $}


%clean
rm -rf $RPM_BUILD_ROOT


%files
#{$ block files $}
%defattr(-,redhawk,redhawk,-)
%dir %{_prefix}/{{component.sdrpath}}/%{name}
#{$ for xmlfile in component.profile.values() $}
%{_prefix}/{{component.sdrpath}}/%{name}/{{xmlfile}}
#{$ endfor $}
#{$ for impl in component.implementations $}
%{_prefix}/{{component.sdrpath}}/%{name}/{{impl.outputdir}}
#{$ endfor $}
#{$ endblock $}

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

Name:           GPP
Version:        1.10.1
Release:        3%{?dist}
Summary:        REDHAWK GPP

Group:          Applications/Engineering
License:        LGPLv3+
URL:            http://redhawksdr.org/
Source:         %{name}-%{version}.tar.gz
Vendor:         REDHAWK

BuildArch:      noarch
BuildRoot:      %{_tmppath}/%{name}-root

Requires:       redhawk >= 1.9
BuildRequires:  redhawk-devel >= 1.9

%package profile
Summary:        Basic GPP profile
Group:          Redhawk/Framework
Prereq:         redhawk >= 1.9
Prereq:         %{name} = %{version}-%{release}

%description
A device representing a general purpose processor
 * Commit: __REVISION__
 * Source Date/Time: __DATETIME__


%description profile
Generates a GPP profile on the installation machine


%prep
%setup


%build
# Implementation DCE:35406744-52f8-4fed-aded-0bcd3aae362b
pushd python
./reconf
%define _bindir %{_prefix}/dev/devices/GPP/python
%configure
make %{?_smp_mflags}
popd


%install
rm -rf $RPM_BUILD_ROOT
# Implementation DCE:35406744-52f8-4fed-aded-0bcd3aae362b
pushd python
%define _bindir %{_prefix}/dev/devices/GPP/python
make install DESTDIR=$RPM_BUILD_ROOT
popd


%clean
rm -rf $RPM_BUILD_ROOT


%files
%defattr(-,redhawk,redhawk,-)
%dir %{_prefix}/dev/devices/%{name}
%{_prefix}/dev/devices/%{name}/GPP.spd.xml
%{_prefix}/dev/devices/%{name}/GPP.prf.xml
%{_prefix}/dev/devices/%{name}/GPP.scd.xml
%{_prefix}/dev/devices/%{name}/python

%files profile
# GPP-profile doesn't install any files


%post profile
# Source profile script for architecture, if available
[ -e /etc/profile.d/redhawk.sh ] && source /etc/profile.d/redhawk.sh

# configure the gpp and the dcd
echo "Configuring the Node..."
%{_prefix}/dev/devices/%{name}/python/nodeconfig.py --silent \
    --clean \
    --gpppath=/devices/%{name} \
    --disableevents \
    --domainname=REDHAWK_DEV \
    --sdrroot=%{_prefix} \
    --inplace


%changelog
* Fri May 24 2013 1.9.0-1
- Remove obsoletes used to upgrade from 1.7.X to 1.8.X
- Update dependencies

* Fri Mar 29 2012 1.8.4-1
- Remove unnecessary defines
- Correct required version of Redhawk; needs at least 1.8.3 due to eventing

* Tue Mar 12 2012 1.8.3-4
- Update licensing information
- Add URL for website
- Change group to a standard one, per Fedora


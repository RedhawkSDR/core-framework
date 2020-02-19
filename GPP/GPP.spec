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
Version:        2.2.6
Release:        1%{?dist}
Summary:        REDHAWK GPP

Group:          Applications/Engineering
License:        LGPLv3+
URL:            http://redhawksdr.org/
Source:         %{name}-%{version}.tar.gz
Vendor:         REDHAWK

Requires(post): redhawk = %{version}
BuildRequires:  redhawk-devel = %{version}
BuildRequires:  numactl-devel >= 2.0
Obsoletes:      %{name} < 2.0


%package profile
Summary:        Basic GPP profile
Group:          Redhawk/Framework
Requires(post): redhawk >= 2.0
Requires(post): redhawk-sdrroot-dev-mgr >= 2.0
Requires(post): %{name} = %{version}-%{release}
Obsoletes:      %{name}-profile < 2.0

%description
A device representing a general purpose processor
 * Commit: __REVISION__
 * Source Date/Time: __DATETIME__


%description profile
Generates a GPP profile on the installation machine


%prep
%setup


%build
pushd cpp
./reconf
%define _bindir %{_prefix}/dev/devices/GPP/cpp
%configure
make %{?_smp_mflags}
popd


%install
rm -rf $RPM_BUILD_ROOT
pushd cpp
%define _bindir %{_prefix}/dev/devices/GPP/cpp
make install DESTDIR=$RPM_BUILD_ROOT
popd


%clean
rm -rf $RPM_BUILD_ROOT


%files
%defattr(-,redhawk,redhawk,-)
%dir %{_prefix}/dev/devices/%{name}
%{_prefix}/dev/devices/%{name}/GPP.spd.xml
%config %{_prefix}/dev/devices/%{name}/GPP.prf.xml
%{_prefix}/dev/devices/%{name}/GPP.scd.xml
%{_prefix}/dev/devices/%{name}/cpp

%files profile
# GPP-profile doesn't install any files

%post
# Source profile script for architecture, if available
[ -e /etc/profile.d/redhawk.sh ] && source /etc/profile.d/redhawk.sh
/sbin/runuser redhawk -s /bin/bash -c '%{_prefix}/dev/devices/%{name}/cpp/gpp_setup --silent \
    --gppcfg \
    --location=%{_prefix}/dev/devices/%{name}' || :

%post profile
# Source profile script for architecture, if available
[ -e /etc/profile.d/redhawk.sh ] && source /etc/profile.d/redhawk.sh

# nodeconfig was previously run as root; correct permissions on these directories
find %{_prefix}/dev/nodes -type d -name 'DevMgr_*' -uid 0 -exec chown -R redhawk. {} \;
/sbin/runuser redhawk -s /bin/bash -c '%{_prefix}/dev/devices/%{name}/cpp/gpp_setup --silent \
    --clean \
    --nodecfg \
    --gpppath=/devices/%{name} \
    --domainname=REDHAWK_DEV \
    --sdrroot=%{_prefix} \
    --inplace' || :


%changelog
* Wed Jun 28 2017 Ryan Bauman <rbauman@lgsinnovations.com> - 2.1.2-1
- Update for 2.1.2-rc1

* Wed Jun 28 2017 Ryan Bauman <rbauman@lgsinnovations.com> - 2.1.1-2
- Bump for 2.1.1-rc2

* Fri Jan 9 2015 1.11.0-1
- Update for cpp GPP

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


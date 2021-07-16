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

Name:           digitizer
Version:        1.0.0
Release:        1%{?dist}
Summary:        Device %{name}

Group:          REDHAWK/Devices
License:        None
Source0:        %{name}-%{version}.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires:  redhawk-devel >= 3.0
Requires:       redhawk >= 3.0


# Interface requirements
BuildRequires:  frontendInterfaces >= 3.0 bulkioInterfaces >= 3.0
Requires:       frontendInterfaces >= 3.0 bulkioInterfaces >= 3.0

BuildArch: noarch


%description
Device %{name}
 * Commit: __REVISION__
 * Source Date/Time: __DATETIME__


%prep
%setup -q


%build
# Implementation python
pushd python
./reconf
%define _bindir %{_prefix}/dev/devices/digitizer/python
%configure
make %{?_smp_mflags}
popd


%install
rm -rf $RPM_BUILD_ROOT
# Implementation python
pushd python
%define _bindir %{_prefix}/dev/devices/digitizer/python
make install DESTDIR=$RPM_BUILD_ROOT
popd


%clean
rm -rf $RPM_BUILD_ROOT


%files
%defattr(-,redhawk,redhawk,-)
%dir %{_sdrroot}/dev/devices/digitizer
%{_prefix}/dev/devices/digitizer/digitizer.spd.xml
%{_prefix}/dev/devices/digitizer/digitizer.scd.xml
%{_prefix}/dev/devices/digitizer/digitizer.prf.xml
%{_prefix}/dev/devices/digitizer/RDC.spd.xml
%{_prefix}/dev/devices/digitizer/RDC.prf.xml
%{_prefix}/dev/devices/digitizer/RDC.scd.xml
%{_prefix}/dev/devices/digitizer/SRDC.spd.xml
%{_prefix}/dev/devices/digitizer/SRDC.prf.xml
%{_prefix}/dev/devices/digitizer/SRDC.scd.xml
%{_prefix}/dev/devices/digitizer/python


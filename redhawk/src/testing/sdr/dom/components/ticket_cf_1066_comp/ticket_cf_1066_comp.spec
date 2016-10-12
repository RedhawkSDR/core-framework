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

Name: ticket_cf_1066_comp
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

BuildArch: noarch

# Python requirements
Requires: python omniORBpy
BuildRequires: libomniORBpy3-devel
BuildRequires: python-devel >= 2.3


%description
Component %{name}


%prep
%setup


%build
# Implementation python
pushd python
./reconf
%define _bindir %{_prefix}/dom/components/ticket_cf_1066_comp/python
%configure
make %{?_smp_mflags}
popd


%install
rm -rf $RPM_BUILD_ROOT
# Implementation python
pushd python
%define _bindir %{_prefix}/dom/components/ticket_cf_1066_comp/python 
make install DESTDIR=$RPM_BUILD_ROOT
popd


%clean
rm -rf $RPM_BUILD_ROOT


%files
%defattr(-,redhawk,redhawk,-)
%dir %{_prefix}/dom/components/%{name}
%{_prefix}/dom/components/%{name}/ticket_cf_1066_comp.spd.xml
%{_prefix}/dom/components/%{name}/ticket_cf_1066_comp.prf.xml
%{_prefix}/dom/components/%{name}/ticket_cf_1066_comp.scd.xml
%{_prefix}/dom/components/%{name}/python
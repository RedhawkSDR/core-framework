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

Name:           TestJavaOptionalProps
Version:        1.0.0
Release:        1%{?dist}
Summary:        Component %{name}

Group:          REDHAWK/Components
License:        None
Source0:        %{name}-%{version}.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires:  redhawk-devel >= 1.11
Requires:       redhawk >= 1.11

BuildArch: noarch

# Java requirements
Requires: java >= 1.6
BuildRequires: java-devel >= 1.6


%description
Component %{name}


%prep
%setup -q


%build
# Implementation java
pushd java
./reconf
%define _bindir %{_prefix}/dom/components/TestJavaOptionalProps/java
%configure
make %{?_smp_mflags}
popd


%install
rm -rf $RPM_BUILD_ROOT
# Implementation java
pushd java
%define _bindir %{_prefix}/dom/components/TestJavaOptionalProps/java
make install DESTDIR=$RPM_BUILD_ROOT
popd


%clean
rm -rf $RPM_BUILD_ROOT


%files
%defattr(-,redhawk,redhawk,-)
%dir %{_prefix}/dom/components/%{name}
%{_prefix}/dom/components/%{name}/TestJavaOptionalProps.scd.xml
%{_prefix}/dom/components/%{name}/TestJavaOptionalProps.prf.xml
%{_prefix}/dom/components/%{name}/TestJavaOptionalProps.spd.xml
%{_prefix}/dom/components/%{name}/java


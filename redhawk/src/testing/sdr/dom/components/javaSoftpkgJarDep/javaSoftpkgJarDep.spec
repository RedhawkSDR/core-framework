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

Name:           javaSoftpkgJarDep
Version:        1.0.0
Release:        1%{?dist}
Summary:        Component %{name}

Group:          REDHAWK/Components
License:        None
Source0:        %{name}-%{version}.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires:  redhawk-devel >= 2.0
Requires:       redhawk >= 2.0

BuildRequires:  java_dep1-devel
Requires:       java_dep1

BuildArch: noarch

# Implementation java
Requires: java >= 1.6
BuildRequires: java-devel >= 1.6

%description
Component %{name}
 * Commit: __REVISION__
 * Source Date/Time: __DATETIME__


%prep
%setup -q


%build
# Implementation java
pushd java
./reconf
%define _bindir %{_prefix}/dom/components/javaSoftpkgJarDep/java
%configure
make %{?_smp_mflags}
popd


%install
rm -rf $RPM_BUILD_ROOT
# Implementation java
pushd java
%define _bindir %{_prefix}/dom/components/javaSoftpkgJarDep/java
make install DESTDIR=$RPM_BUILD_ROOT
popd


%clean
rm -rf $RPM_BUILD_ROOT


%files
%defattr(-,redhawk,redhawk,-)
%dir %{_sdrroot}/dom/components/javaSoftpkgJarDep
%{_prefix}/dom/components/javaSoftpkgJarDep/javaSoftpkgJarDep.scd.xml
%{_prefix}/dom/components/javaSoftpkgJarDep/javaSoftpkgJarDep.prf.xml
%{_prefix}/dom/components/javaSoftpkgJarDep/javaSoftpkgJarDep.spd.xml
%{_prefix}/dom/components/javaSoftpkgJarDep/java


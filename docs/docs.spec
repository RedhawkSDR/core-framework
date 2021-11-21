Name:           redhawk-docs
Version:        3.0.0
Release:        1%{?dist}
Summary:        The Redhawk Manual

License:        LGPLv3+
Group:          Applications/Engineering
URL:            https://redhawksdr.org
Source:         %{name}-%{version}.tar.gz
Vendor:         REDHAWK

BuildArch:      noarch

Requires:       redhawk >= 3.0.0
BuildRequires:  pandoc
BuildRequires:  python3

%description
REDHAWK Manual
 * Commit: __REVISION__
 * Source Date/Time: __DATETIME__

%prep
%setup -q

%build
make

%install
mkdir -p ${buildroot}/usr/local/redhawk/core/docs
cp -dr css font html img js md README.md ${buildroot}/usr/local/redhawk/core/docs/

%files
%{_prefix}/usr/local/redhawk/core/docs
%{_prefix}/usr/local/redhawk/core/docs/font
%{_prefix}/usr/local/redhawk/core/docs/html
%{_prefix}/usr/local/redhawk/core/docs/img
%{_prefix}/usr/local/redhawk/core/docs/js
%{_prefix}/usr/local/redhawk/core/docs/md
%{_prefix}/usr/local/redhawk/core/docs/py
%{_prefix}/usr/local/redhawk/core/docs/README*


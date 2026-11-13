%global src_version 0.11.0
%global src_dir logging-log4cxx-%{src_version}

Name:           log4cxx
Version:        0.11.0
Release:        1%{?dist}
Summary:        Apache log4cxx C++ logging library (pinned for REDHAWK)

License:        ASL 2.0
URL:            https://logging.apache.org/log4cxx/
# REDHAWK links against this specific release (ABI soname liblog4cxx.so.11).
# It is built and version-pinned here so the REDHAWK RPMs have a provider for
# their auto-generated soname dependency instead of an unpackaged source build.
Source0:        https://github.com/apache/logging-log4cxx/archive/refs/tags/v%{src_version}.tar.gz

BuildRequires:  cmake
BuildRequires:  gcc-c++
BuildRequires:  make
BuildRequires:  apr-devel
BuildRequires:  apr-util-devel
BuildRequires:  expat-devel

%description
Apache log4cxx %{version}, built from the upstream release that REDHAWK is
developed and tested against.

%package devel
Summary:        Development files for %{name}
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       apr-devel
Requires:       apr-util-devel

%description devel
Headers, pkg-config, and CMake files for Apache log4cxx %{version}.

%prep
%setup -q -n %{src_dir}

%build
# BUILD_TESTING=OFF avoids the test suite's download of log4j (from a dead
# Apache mirror) and keeps the build network-free.
%cmake -DBUILD_SHARED_LIBS=ON -DBUILD_TESTING=OFF
%cmake_build

%install
%cmake_install

%files
%{_libdir}/liblog4cxx.so.*

%files devel
%{_includedir}/log4cxx/
%{_libdir}/liblog4cxx.so
%{_datadir}/pkgconfig/liblog4cxx.pc
%{_datadir}/cmake/log4cxx/

%changelog
* Sun Oct 19 2025 REDHAWK <redhawk@redhawksdr.org> - 0.11.0-1
- Package the pinned log4cxx release used by REDHAWK core-framework.

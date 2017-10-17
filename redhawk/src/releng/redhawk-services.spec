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
%if 0%{?fedora} >= 17 || 0%{?rhel} >=7
%global with_systemd 1
%endif
%{!?_ossiehome:  %define _ossiehome  /usr/local/redhawk/core}
%{!?_sdrroot:    %define _sdrroot    /var/redhawk/sdr}
%define _prefix %{_ossiehome}
Prefix:         %{_ossiehome}
Prefix:         %{_sdrroot}
Prefix:         %{_sysconfdir}

Name:           redhawk-services
Version:        2.1.2
Release:        2%{?dist}
Summary:        REDHAWK System Services

Group:          Applications/Engineering
License:        LGPLv3+
URL:            http://redhawksdr.org/
Vendor:         REDHAWK
BuildArch:      noarch
BuildRoot:      %{_builddir}/%{name}-root
Source0:        redhawk-services-%{version}.tar.gz

Requires:       bash
Requires:       crudini
Requires:       binutils
Requires:        python
Requires:       redhawk >= 2.0
Requires:       redhawk-sdrroot-dom-mgr >= 2.0
Requires:       redhawk-sdrroot-dev-mgr >= 2.0

%if 0%{?with_systemd}
%{?systemd_requires}
BuildRequires:   systemd
Requires:        systemd
Requires:        systemctl
Requires(pre):   systemd
Requires(pre):   systemctl
%else
Requires:       /sbin/service /sbin/chkconfig
Requires(pre):  /sbin/service
%endif

%description
Service scripts for REDHAWK, a Software Defined Radio framework.
 * Commit: __REVISION__
 * Source Date/Time: __DATETIME__

%install
rm -rf --preserve-root $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/etc
tar -C $RPM_BUILD_ROOT/etc -xf %{SOURCE0}
echo "REDHAWK_VERSION=%{version}-%{release}" > $RPM_BUILD_ROOT/etc/redhawk/redhawk-release

%if 0%{?with_systemd}
mkdir -p %{buildroot}%{_unitdir}
install -m 0644 %{buildroot}/etc/redhawk/init.d/redhawk-domain-mgrs.service %{buildroot}%{_unitdir}
install -m 0644 %{buildroot}/etc/redhawk/init.d/redhawk-device-mgrs.service %{buildroot}%{_unitdir}
install -m 0644 %{buildroot}/etc/redhawk/init.d/redhawk-waveforms.service %{buildroot}%{_unitdir}
install -m 0644 %{buildroot}/etc/redhawk/init.d/redhawk-domain-mgr@.service %{buildroot}%{_unitdir}
install -m 0644 %{buildroot}/etc/redhawk/init.d/redhawk-device-mgr@.service %{buildroot}%{_unitdir}
install -m 0644 %{buildroot}/etc/redhawk/init.d/redhawk-waveform@.service %{buildroot}%{_unitdir}
%endif
install -d 0777 -g redhawk %{buildroot}/%{_localstatedir}/run/redhawk
install -d 0777 -g redhawk %{buildroot}/%{_localstatedir}/lock/redhawk
install -d 0777 -g redhawk %{buildroot}/%{_localstatedir}/lock/redhawk/domain-mgrs
install -d 0777 -g redhawk %{buildroot}/%{_localstatedir}/lock/redhawk/device-mgrs
install -d 0777 -g redhawk %{buildroot}/%{_localstatedir}/lock/redhawk/waveforms
install -d 0777 -g redhawk %{buildroot}/%{_localstatedir}/log/redhawk
install -d 0777 -g redhawk %{buildroot}/%{_localstatedir}/run/redhawk
install -d 0777 -g redhawk %{buildroot}/%{_localstatedir}/run/redhawk/domain-mgrs
install -d 0777 -g redhawk %{buildroot}/%{_localstatedir}/run/redhawk/device-mgrs
install -d 0777 -g redhawk %{buildroot}/%{_localstatedir}/run/redhawk/waveforms

%clean
rm -rf --preserve-root $RPM_BUILD_ROOT


%pre
# -r is system account, -f is force (ignore already exists)
groupadd -r -f redhawk
if ! id redhawk &> /dev/null; then
  # -M is don't create home dir, -r is system account, -s is shell
  # -c is comment, -n is don't create group, -g is group name/id
  /usr/sbin/useradd -M -r -s /sbin/nologin \
    -c "REDHAWK System Account" -n -g redhawk redhawk > /dev/null
fi

%files
%defattr(664,root,redhawk,775)
%dir %attr(775,root,redhawk) %{_sysconfdir}/redhawk
%attr(444,root,redhawk) %{_sysconfdir}/redhawk/redhawk-release
%attr(664,root,redhawk) %{_sysconfdir}/redhawk/rh.cond.cfg
%dir %attr(775,root,redhawk) %{_sysconfdir}/redhawk/cron.d
%attr(664,root,redhawk) %{_sysconfdir}/redhawk/cron.d/redhawk
%dir %attr(775,root,redhawk) %{_sysconfdir}/redhawk/domains.d
%attr(664,root,redhawk) %{_sysconfdir}/redhawk/domains.d/example.domains
%dir %attr(775,root,redhawk) %{_sysconfdir}/redhawk/logging
%attr(664,root,redhawk) %{_sysconfdir}/redhawk/logging/default.logging.properties
%attr(664,root,redhawk) %{_sysconfdir}/redhawk/logging/example.logging.properties
%attr(664,root,redhawk) %{_sysconfdir}/redhawk/logging/logrotate.redhawk
%dir %attr(775,root,redhawk) %{_sysconfdir}/redhawk/nodes.d
%attr(664,root,redhawk) %{_sysconfdir}/redhawk/nodes.d/example.nodes
%dir %attr(775,root,redhawk) %{_sysconfdir}/redhawk/security
%dir %attr(775,root,redhawk) %{_sysconfdir}/redhawk/security/limits.d
%attr(664,root,redhawk) %{_sysconfdir}/redhawk/security/limits.d/99-redhawk-limits.conf
%dir %attr(775,root,redhawk) %{_sysconfdir}/redhawk/sysctl.d
%attr(664,root,redhawk) %{_sysconfdir}/redhawk/sysctl.d/sysctl.conf
%dir %attr(775,root,redhawk) %{_sysconfdir}/redhawk/waveforms.d
%attr(664,root,redhawk) %{_sysconfdir}/redhawk/waveforms.d/example.waveforms
%dir %attr(775,root,redhawk) %{_sysconfdir}/redhawk/init.d
%attr(755,root,redhawk) %{_sysconfdir}/redhawk/init.d/redhawk-domain-mgrs
%attr(644,root,redhawk) %{_sysconfdir}/redhawk/init.d/redhawk-domain-mgrs.service
%attr(644,root,redhawk) %{_sysconfdir}/redhawk/init.d/redhawk-domain-mgr@.service
%attr(644,root,redhawk) %{_sysconfdir}/redhawk/init.d/redhawk-device-mgrs.service
%attr(644,root,redhawk) %{_sysconfdir}/redhawk/init.d/redhawk-device-mgr@.service
%attr(755,root,redhawk) %{_sysconfdir}/redhawk/init.d/redhawk-device-mgrs
%attr(755,root,redhawk) %{_sysconfdir}/redhawk/init.d/redhawk-waveforms
%attr(755,root,redhawk) %{_sysconfdir}/redhawk/init.d/redhawk-waveform
%attr(644,root,redhawk) %{_sysconfdir}/redhawk/init.d/redhawk-waveform@.service
%attr(644,root,redhawk) %{_sysconfdir}/redhawk/init.d/redhawk-waveforms.service
%attr(755,root,redhawk) %{_sysconfdir}/redhawk/init.d/omni-clean
%attr(755,root,redhawk) %{_sysconfdir}/redhawk/init.d/redhawk-service-config
%attr(755,root,redhawk) %{_sysconfdir}/redhawk/init.d/redhawk-service-list
%attr(755,root,redhawk) %{_sysconfdir}/redhawk/init.d/redhawk-service-start
%attr(755,root,redhawk) %{_sysconfdir}/redhawk/init.d/redhawk-service-status
%attr(755,root,redhawk) %{_sysconfdir}/redhawk/init.d/redhawk-service-stop
%attr(755,root,redhawk) %{_sysconfdir}/redhawk/init.d/redhawk-wf-control
%attr(775,root,redhawk) %{_sysconfdir}/redhawk/init.d/conditional_overrides
%attr(755,root,redhawk) %{_sysconfdir}/redhawk/init.d/functions
%attr(755,root,redhawk) %{_sysconfdir}/redhawk/init.d/svc-functions
%attr(755,root,redhawk) %{_sysconfdir}/redhawk/init.d/rundir
%attr(755,root,redhawk) %{_sysconfdir}/redhawk/init.d/set-env
%attr(664,root,redhawk) %{_sysconfdir}/redhawk/init.d/*.defaults
%dir %attr(0777,root,redhawk) %{_localstatedir}/log/redhawk
%dir %attr(0777,root,redhawk) %{_localstatedir}/lock/redhawk
%dir %attr(0777,root,redhawk) %{_localstatedir}/lock/redhawk
%dir %attr(0777,root,redhawk) %{_localstatedir}/lock/redhawk/domain-mgrs
%dir %attr(0777,root,redhawk) %{_localstatedir}/lock/redhawk/device-mgrs
%dir %attr(0777,root,redhawk) %{_localstatedir}/lock/redhawk/waveforms
%dir %attr(0777,root,redhawk) %{_localstatedir}/run/redhawk
%dir %attr(0777,root,redhawk) %{_localstatedir}/run/redhawk/domain-mgrs
%dir %attr(0777,root,redhawk) %{_localstatedir}/run/redhawk/device-mgrs
%dir %attr(0777,root,redhawk) %{_localstatedir}/run/redhawk/waveforms
%if 0%{?with_systemd}
%defattr (-,root,root)
%{_unitdir}/redhawk-domain-mgrs.service
%{_unitdir}/redhawk-device-mgrs.service
%{_unitdir}/redhawk-waveforms.service
%{_unitdir}/redhawk-domain-mgr@.service
%{_unitdir}/redhawk-device-mgr@.service
%{_unitdir}/redhawk-waveform@.service
%endif

%post

cp %{_sysconfdir}/redhawk/cron.d/redhawk %{_sysconfdir}/cron.d

%if 0%{?with_systemd}

%systemd_post redhawk-domain-mgrs.service
%systemd_post redhawk-device-mgrs.service
%systemd_post redhawk-waveforms.service

systemctl reload crond > /dev/null 2>&1 || :

%else
ln -s %{_sysconfdir}/redhawk/init.d/redhawk-domain-mgrs %{_sysconfdir}/rc.d/init.d/
ln -s %{_sysconfdir}/redhawk/init.d/redhawk-device-mgrs %{_sysconfdir}/rc.d/init.d/
ln -s %{_sysconfdir}/redhawk/init.d/redhawk-waveforms %{_sysconfdir}/rc.d/init.d/

/sbin/chkconfig --add redhawk-domain-mgrs
/sbin/chkconfig --add redhawk-device-mgrs
/sbin/chkconfig --add redhawk-waveforms

service crond reload > /dev/null 2>&1 || :

%endif

%preun

%if 0%{?with_systemd}

%systemd_preun redhawk-domain-mgrs.service
%systemd_preun redhawk-device-mgrs.service
%systemd_preun redhawk-waveforms.service

%else

/sbin/service redhawk-waveforms stop > /dev/null 2>&1 || :
/sbin/service redhawk-device-mgrs stop > /dev/null 2>&1 || :
/sbin/service redhawk-domain-mgrs stop > /dev/null 2>&1 || :

/sbin/chkconfig --del redhawk-domain-mgrs  > /dev/null 2>&1 || :
/sbin/chkconfig --del redhawk-device-mgrs  > /dev/null 2>&1 || :
/sbin/chkconfig --del redhawk-waveforms  > /dev/null 2>&1 || :

%endif


%postun

[ -f %{_sysconfdir}/cron.d/redhawk ] && rm -f  %{_sysconfdir}/cron.d/redhawk || :


%if 0%{?with_systemd}

%systemd_postun_with_restart redhawk-domain-mgrs.service
%systemd_postun_with_restart redhawk-device-mgrs.service
%systemd_postun_with_restart redhawk-waveforms.service

systemctl reload crond  > /dev/null 2>&1 || :

%else

[ -h %{_sysconfdir}/init.d/redhawk-domain-mgrs ] && rm -f %{_sysconfdir}/init.d/redhawk-domain-mgrs
[ -h %{_sysconfdir}/init.d/redhawk-device-mgrs ] && rm -f %{_sysconfdir}/init.d/redhawk-device-mgrs
[ -h %{_sysconfdir}/init.d/redhawk-waveforms ] && rm -f %{_sysconfdir}/init.d/redhawk-waveforms

/sbin/service crond reload  > /dev/null 2>&1 || :

%endif


%changelog



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

# Update etc scripts with appropriate paths
# This setup for scripts taken from http://www.gnu.org/software/autoconf/manual/autoconf.html, section 4.8.2 (Installation Directory Variables)

ldsoconfddir = $(sysconfdir)/ld.so.conf.d
ldsoconfd_DATA = ld.so.conf.d/redhawk.conf

profileddir = $(sysconfdir)/profile.d
profiled_DATA = profile.d/redhawk.sh \
  profile.d/redhawk.csh \
  profile.d/redhawk-sdrroot.sh \
  profile.d/redhawk-sdrroot.csh \
  profile.d/local_pkg_config.sh \
  profile.d/local_pkg_config.csh

lib = lib${gr_libdir_suffix}
edit = $(SED) \
  -e 's|@prefix[@]|$(prefix)|g' \
  -e 's|@bindir[@]|$(bindir)|g' \
  -e 's|@libdir[@]|$(libdir)|g' \
  -e 's|@sdrroot[@]|$(SDR_ROOT)|g' \
  -e 's|@lib[@]|$(lib)|g'

$(ldsoconfd_DATA) $(profiled_DATA): Makefile
	$(edit) $@.in > $@

ld.so.conf.d/redhawk.conf: ld.so.conf.d/redhawk.conf.in
profile.d/redhawk.sh: profile.d/redhawk.sh.in
profile.d/redhawk.csh: profile.d/redhawk.csh.in
profile.d/redhawk-sdrroot.sh: profile.d/redhawk-sdrroot.sh.in
profile.d/redhawk-sdrroot.csh: profile.d/redhawk-sdrroot.csh.in
profile.d/local_pkg_config.sh: profile.d/local_pkg_config.sh.in
profile.d/local_pkg_config.csh: profile.d/local_pkg_config.csh.in

DISTCLEANFILES = $(ldsoconfd_DATA) $(profiled_DATA)


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

# vim: noet: softtabstop=0
domdir = $(SDR_ROOT)/dom
domaindir = $(domdir)/domain
componentsdir = $(domdir)/components
waveformsdir = $(domdir)/waveforms
devdir = $(SDR_ROOT)/dev
devicesdir = $(devdir)/devices
nodesdir = $(devdir)/nodes

dist_domain_DATA = domain/DomainManager.dmd.xml.template

SUBDIRS = dommgr devmgr

install-exec-hook:
	$(mkdir_p) $(DESTDIR)$(domaindir)
	$(mkdir_p) $(DESTDIR)$(componentsdir)
	$(mkdir_p) $(DESTDIR)$(waveformsdir)
	$(mkdir_p) $(DESTDIR)$(devicesdir)
	$(mkdir_p) $(DESTDIR)$(nodesdir)

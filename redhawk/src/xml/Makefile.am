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

dtddir = $(datadir)/xml/sca/2.2.2/dtd
xsddir = $(datadir)/xml/sca/2.2.2/xsd
dist_dtd_DATA = dtd/deviceconfiguration.dtd \
                dtd/devicepkg.dtd \
		dtd/domainmanagerconfiguration.dtd \
		dtd/profile.dtd \
		dtd/properties.dtd \
		dtd/softpkg.dtd \
		dtd/softwareassembly.dtd \
		dtd/softwarecomponent.dtd \
		dtd/catalog.xml

dist_xsd_DATA = xsd/dcd.xsd \
                xsd/dmd.xsd \
		xsd/dpd.xsd \
		xsd/prf.xsd \
		xsd/profile.xsd \
		xsd/sad.xsd \
		xsd/scd.xsd \
		xsd/spd.xsd

if HAVE_XMLCATALOG
install-exec-hook:
	-xmlcatalog --noout --add delegatePublic "-//JTRS//DTD SCA V2.2.2" "$(datadir)/xml/sca/2.2.2/dtd/catalog.xml" /etc/xml/catalog

endif

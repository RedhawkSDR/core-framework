# This file is protected by Copyright. Please refer to the COPYRIGHT file
# distributed with this source distribution.
#
# This file is part of Geon's Docker REDHAWK.
#
# Docker REDHAWK is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
#
# Docker REDHAWK is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see http://www.gnu.org/licenses/.
#

FROM geontech/redhawk-base:VERSION
LABEL name="REDHAWK SDR Runtime" \
    description="REDHAWK SDR Runtime dependencies"

# Install REDHAWK Runtime, no GPP, domain, or boot scripts.
RUN yum groupinstall -y "REDHAWK Runtime" && \
	cp /etc/profile.d/redhawk-sdrroot.sh /etc/profile.d/redhawk-sdrroot.sh.bak && \
	yum remove -y \
		GPP \
		GPP-profile \
		omniEvents-bootscripts \
		redhawk-sdrroot-dev-mgr \
		redhawk-sdrroot-dom-mgr \
		redhawk-sdrroot-dom-profile && \
	yum clean all -y && \
	mv /etc/profile.d/redhawk-sdrroot.sh.bak /etc/profile.d/redhawk-sdrroot.sh

# Polling scripts for dependencies on omniserver
ADD files/wait-for-eventchannel /usr/local/bin/wait-for-eventchannel
ADD files/wait-for-domain       /usr/local/bin/wait-for-domain
RUN chmod u+x /usr/local/bin/wait-for-*

CMD ["/bin/bash", "-l"]

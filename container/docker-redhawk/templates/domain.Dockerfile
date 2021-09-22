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

FROM geontech/redhawk-runtime:VERSION
LABEL name="REDHAWK SDR Domain" \
    description="REDHAWK SDR Domain Runner"

RUN yum install -y \
		redhawk-basic-components \
		redhawk-sdrroot-dom-mgr \
		redhawk-sdrroot-dom-profile && \
	yum clean all -y

VOLUME /var/redhawk/sdr

ENV DOMAINNAME ""

# Supervisord configuration script and "exit" listener
ADD files/supervisord-domain.conf /etc/supervisor.d/domain.conf
ADD files/kill_supervisor.py /usr/bin/kill_supervisor.py
RUN chmod ug+x /usr/bin/kill_supervisor.py

CMD ["/usr/bin/supervisord"]

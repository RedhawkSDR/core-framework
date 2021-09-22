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

LABEL name="OmniORB Servers" \
    description="Omni* Services Runner"

# Create log directories and add supervisord config for omni.
RUN mkdir -p /var/log/omniORB && \
	mkdir -p /var/log/omniEvents
ADD files/supervisord-omniserver.conf /etc/supervisor.d/omniserver.conf
ADD files/kill_supervisor.py /usr/bin/kill_supervisor.py
RUN chmod u+x /usr/bin/kill_supervisor.py

EXPOSE 2809 11169

CMD ["/usr/bin/supervisord"]

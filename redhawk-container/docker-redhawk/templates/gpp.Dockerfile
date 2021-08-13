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
LABEL name="REDHAWK GPP Device" \
    description="REDHAWK GPP Runner"

ENV DOMAINNAME ""
ENV NODENAME   ""
ENV GPPNAME    ""

RUN yum install -y \
      fftw \
      GPP \
      GPP-profile && \
    yum clean all -y

# Create the node init script for the GPP
ADD files/gpp-node-init.sh /root/gpp-node-init.sh
RUN chmod u+x /root/gpp-node-init.sh && \
    echo "source /root/gpp-node-init.sh" | tee -a /root/.bashrc

# Create the supervisord device script and "exit" event listener
ADD files/supervisord-device.conf /etc/supervisor.d/device.conf
ADD files/kill_supervisor.py /usr/bin/kill_supervisor.py
RUN chmod ug+x /usr/bin/kill_supervisor.py

CMD ["/usr/bin/supervisord"]

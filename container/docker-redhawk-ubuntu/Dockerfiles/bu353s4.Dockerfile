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

FROM geontech/redhawk-ubuntu-runtime:2.0.6
LABEL name="Geon Technology's BU353S4 GPS Device" \
    description="Geon's BU353S4"

ENV DOMAINNAME ""
ENV GPS_PORT   ""
ENV NODENAME   "" 

RUN apt-get update && \
    apt-get install -qy --no-install-recommends \
        libusb-1.0.0 && \
    rm -rf /var/lib/apt/lists/*

# Add dependencies scripts and builder script.
# Run the builder.
WORKDIR /tmp/build
ADD files/build/base-deps-func.sh \
    files/build/redhawk-source-repo-func.sh \
    files/build/build-sh-process-func.sh \
    files/build/bu353s4.sh \
    ./
RUN bash bu353s4.sh && rm *

# Add script for configuring the node
ADD files/init/bu353s4-node-init.sh /root/bu353s4-node-init.sh
RUN chmod u+x /root/bu353s4-node-init.sh && \
    echo "/root/bu353s4-node-init.sh" | tee -a /root/.bashrc

# BU353S4 Supervisord script and 'exit' event listener
ADD files/supervisor/supervisord-bu353s4.conf /etc/supervisor.d/bu353s4.conf
ADD files/supervisor/kill_supervisor.py /usr/bin/kill_supervisor.py
RUN chmod ug+x /usr/bin/kill_supervisor.py

WORKDIR /root
CMD ["supervisord"]

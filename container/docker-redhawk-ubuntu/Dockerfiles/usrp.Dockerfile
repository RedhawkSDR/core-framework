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
LABEL name="REDHAWK SDR USRP_UHD Device" \
    description="REDHAWK USRP_UHD w/ updated UHD driver version (3.10)"

ENV DOMAINNAME      ""
ENV NODENAME        ""
ENV USRP_IP_ADDRESS ""
ENV USRP_TYPE       ""
ENV USRP_NAME       ""
ENV USRP_SERIAL     ""

# Add dependencies scripts and builder script.
# Run the builder.
WORKDIR /tmp/build
ADD files/build/base-deps-func.sh \
    files/build/redhawk-source-repo-func.sh \
    files/build/build-sh-process-func.sh \
    files/build/uhd-func.sh \
    files/build/usrp.sh \
    ./
RUN bash usrp.sh && rm *

# Add script for configuring the node
ADD files/init/usrp-node-init.sh /root/usrp-node-init.sh
RUN chmod u+x /root/usrp-node-init.sh && \
    echo "/root/usrp-node-init.sh" | tee -a /root/.bashrc

# USRP Supervisord script and exit script
ADD files/supervisor/supervisord-usrp.conf /etc/supervisor.d/usrp.conf
ADD files/supervisor/kill_supervisor.py /usr/bin/kill_supervisor.py
RUN chmod u+x /usr/bin/kill_supervisor.py

WORKDIR /root
CMD ["supervisord"]

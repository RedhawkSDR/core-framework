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
LABEL name="REDHAWK SDR USRP_UHD Device" \
    description="REDHAWK USRP_UHD w/ updated UHD driver version (3.10)"

# Compile UHD from source
RUN yum install -y \
        redhawk-sdrroot-dev-mgr \
        python-requests \
        rh.USRP_UHD && \
    yum clean all -y && \
    /usr/bin/uhd_images_downloader

ENV DOMAINNAME      ""
ENV NODENAME        ""
ENV NODEFLAGS       ""
ENV USRP_IP_ADDRESS ""
ENV USRP_TYPE       ""
ENV USRP_NAME       ""
ENV USRP_SERIAL     ""

# Add script for configuring the node
ADD files/usrp-node-init.sh /root/usrp-node-init.sh
ADD files/usrp-nodeconfig.patch /root/usrp-nodeconfig.patch
RUN chmod u+x /root/usrp-node-init.sh && \
    echo "source /root/usrp-node-init.sh" | tee -a /root/.bashrc

# USRP Supervisord script and exit script
ADD files/supervisord-usrp.conf /etc/supervisor.d/usrp.conf
ADD files/kill_supervisor.py /usr/bin/kill_supervisor.py
RUN chmod u+x /usr/bin/kill_supervisor.py

CMD ["/usr/bin/supervisord"]

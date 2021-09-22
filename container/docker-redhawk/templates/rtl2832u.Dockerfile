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
LABEL name="REDHAWK SDR RTL2832U Device" \
    description="REDHAWK RTL2832U"

RUN yum install -y \
        redhawk-sdrroot-dev-mgr \
        redhawk-devel \
        libusb1-devel \
        librtlsdr \
        autoconf \
        automake \
        git && \
    \
    source /etc/profile.d/redhawk.sh && \
    source /etc/profile.d/redhawk-sdrroot.sh && \
    git clone git://github.com/RedhawkSDR/RTL2832U.git && \
    pushd RTL2832U && \
    git checkout tags/2.0.1-5 && \
    ./build.sh && \
    ./build.sh install && \
    popd && \
    rm -rf RTL2832U && \
    \
    yum autoremove -y \
    	libusb1-devel \
        redhawk-devel \
        autoconf \
        automake && \
    yum install -y \
        boost-regex \
        libusb1 && \
    yum clean all -y

ENV DOMAINNAME  ""
ENV RTL_NAME    ""
ENV RTL_VENDOR  ""
ENV RTL_PRODUCT ""
ENV RTL_SERIAL  ""
ENV RTL_INDEX   ""
ENV NODENAME    ""
ENV NODEFLAGS   ""

# Add script for configuring the node
ADD files/rtl2832u-node-init.sh /root/rtl2832u-node-init.sh
ADD files/rtl2832u-nodeconfig.patch /root/rtl2832u-nodeconfig.patch
RUN chmod u+x /root/rtl2832u-node-init.sh && \
    echo "source /root/rtl2832u-node-init.sh" | tee -a /root/.bashrc

# RTL2832U Supervisord script
ADD files/supervisord-rtl2832u.conf /etc/supervisor.d/rtl2832u.conf
ADD files/kill_supervisor.py /usr/bin/kill_supervisor.py
RUN chmod u+x /usr/bin/kill_supervisor.py

CMD ["/usr/bin/supervisord"]

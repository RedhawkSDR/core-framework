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
LABEL name="REDHAWK SDR RTL2832U Device" \
    description="REDHAWK RTL2832U"

ENV DOMAINNAME  ""
ENV RTL_NAME    ""
ENV RTL_VENDOR  ""
ENV RTL_PRODUCT ""
ENV RTL_SERIAL  ""
ENV RTL_INDEX   ""
ENV NODENAME    "" 

# Add dependencies scripts and builder script.
# Run the builder.
WORKDIR /tmp/build
ADD files/build/base-deps-func.sh \
    files/build/redhawk-source-repo-func.sh \
    files/build/build-sh-process-func.sh \
    files/build/rtl2832u.sh \
    ./
RUN bash rtl2832u.sh && rm * && \
	apt-get update && \
	apt-get install -qy --no-install-recommends libusb-1.0-0 && \
    apt-get autoremove -qy && \
    rm -rf /var/lib/apt/lists/*

# Add script for configuring the node
ADD files/init/rtl2832u-node-init.sh /root/rtl2832u-node-init.sh
RUN chmod u+x /root/rtl2832u-node-init.sh && \
	echo "/root/rtl2832u-node-init.sh" | tee -a /root/.bashrc

# RTL2832U Supervisord script
ADD files/supervisor/supervisord-rtl2832u.conf /etc/supervisor.d/rtl2832u.conf
ADD files/supervisor/kill_supervisor.py /usr/bin/kill_supervisor.py
RUN chmod u+x /usr/bin/kill_supervisor.py

WORKDIR /root
CMD ["supervisord"]

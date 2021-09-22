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
LABEL name="Geon Technology's BU353S4 GPS Device" \
    description="Geon's BU353S4"

# Yum necessities, download nmealib and bu353s4, compile each, remove unused things
RUN yum install -y \
		redhawk-sdrroot-dev-mgr \
		redhawk-devel \
		libusb1-devel \
        autoconf \
        automake \
        git \
        unzip && \
    \
	wget http://downloads.sourceforge.net/project/nmea/NmeaLib/nmea-0.5.x/nmealib-0.5.3.zip && \
	unzip nmealib-0.5.3.zip && \
	pushd nmealib && \
	make && cp -r lib include /usr/local && \
	popd && rm -rf nmealib nmealib-0.5.3.zip && \
	\
	git clone git://github.com/GeonTech/BU353S4.git && \
	pushd BU353S4 && \
	source /etc/profile.d/redhawk.sh && \
	source /etc/profile.d/redhawk-sdrroot.sh && \
	./build.sh && ./build.sh install && \
	popd && rm -rf BU353S4 && \
	\
	yum autoremove -y \
		redhawk-devel \
		libusb1-devel \
        autoconf \
        automake \
        git \
        unzip && \
    yum install -y \
    	boost-regex \
    	libusb1 && \
    yum clean all -y

ENV DOMAINNAME ""
ENV GPS_PORT   ""
ENV NODENAME   ""

# Set config file to executable
RUN chmod a+x /var/redhawk/sdr/dev/devices/BU353S4/nodeconfig.py

# Add script for configuring the node
ADD files/bu353s4-node-init.sh /root/bu353s4-node-init.sh
RUN chmod u+x /root/bu353s4-node-init.sh && \
	echo "source /root/bu353s4-node-init.sh" | tee -a /root/.bashrc

# BU353S4 Supervisord script and 'exit' event listener
ADD files/supervisord-bu353s4.conf /etc/supervisor.d/bu353s4.conf
ADD files/kill_supervisor.py /usr/bin/kill_supervisor.py
RUN chmod ug+x /usr/bin/kill_supervisor.py

CMD ["/usr/bin/supervisord"]

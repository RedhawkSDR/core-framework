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
LABEL name="REDHAWK IDE Environment" \
	description="REDHAWK Integrated Development Environment Runner"

# Install development environment
RUN yum groupinstall -y "REDHAWK Development" && \
	yum install -y \
		rh.blueFileLib-devel \
		rh.dsp-devel \
		rh.fftlib-devel \
		rh.RedhawkDevUtils-devel \
		rh.VITA49-devel \
		PackageKit-gtk-module \
		libcanberra-gtk2 \
		sudo && \
	yum autoremove -y && \
	yum clean all -y

ADD files/rhide-init.sh /opt/rhide-init.sh
RUN chmod a+x /opt/rhide-init.sh

ENV RHUSER_ID 54321

VOLUME /var/redhawk/sdr
VOLUME /home/user/workspace

# Run the RHIDE init script
CMD ["/bin/bash", "-l", "-c", "/opt/rhide-init.sh"]

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

FROM geontech/redhawk-ubuntu-base:2.0.6
LABEL name="REDHAWK SDR Runtime" \
    description="REDHAWK SDR Runtime dependencies (core framework, etc.)"

# Runtime dependencis
RUN apt-get update && \
	apt-get install -qy --no-install-recommends \
        libexpat1 \
        libuuid1 \
        uuid \
        libfftw3-3 \
        liblog4cxx10v5 \
        python-numpy \
        python-jinja2 \
        libboost-date-time1.58.0 \
        libboost-filesystem1.58.0 \
        libboost-regex1.58.0 \
        libboost-serialization1.58.0 \
        libboost-system1.58.0 \
        libboost-thread1.58.0 \
        libboost-iostreams1.58.0 \
        libapr1 \
        libaprutil1 \
        python-omniorb \
        libcos4-1 \
        default-jdk && \
    rm -rf /var/lib/apt/lists/*

# Add build dependencies and builder script for core framework, etc.
WORKDIR /tmp/build
ADD files/build/base-deps-func.sh \
	files/build/redhawk-source-repo-func.sh \
	files/build/core-framework.sh \
	./
RUN bash core-framework.sh && rm *
RUN echo "source /etc/profile.d/redhawk.sh" >> /etc/bash.bashrc && \
    echo "source /etc/profile.d/redhawk-sdrroot.sh" >> /etc/bash.bashrc

# Add the REDHAWK user and group, own SDRROOT
RUN /usr/sbin/useradd -M -r -s /sbin/nologin \
    -c "REDHAWK System Account" redhawk > /dev/null && \
    chown -R redhawk:redhawk /var/redhawk/sdr && \
    chmod -R g+ws /var/redhawk/sdr

WORKDIR /root

CMD ["/bin/bash", "-l"]

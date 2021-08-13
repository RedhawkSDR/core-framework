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
FROM ubuntu:16.04

ENV RH_VERSION=2.0.6
ENV OMNISERVICEIP 127.0.0.1

LABEL name="REDHAWK SDR Base Image" \
    license="GPLv3" \
    description="REDHAWK SDR base image (omniservice libs, configuration, etc.))" \
    maintainer="Thomas Goodwin <btgoodwin@geontech.com>" \
    version="${RH_VERSION}" \
    vendor="Geon Technologies, LLC"

# Install omniorb name server, pip, and supervisord, default supervisord config
RUN apt-get update && \
    apt-get install -qy --no-install-recommends \
    python \
    omniorb-nameserver \
    omniidl \
    omniidl-python \
    omniorb-idl \
    python-pip \
    libboost-system1.58.0 \
    libboost-thread1.58.0 \
    && \
    pip install --upgrade pip==9.0.3 && \
    pip install --upgrade setuptools && \
    pip install --upgrade supervisor && \
    mkdir -p \
        /etc/supervisor.d    \
        /etc/supervisor      \
        /var/log/supervisord && \
    rm -rf /var/lib/apt/lists/* && \
    sed -i "/\"\$PS1\"/d" /root/.bashrc
ADD files/supervisor/supervisord.conf /etc/supervisor/supervisord.conf

# Add scripts for compiling omni events from source and run them
WORKDIR /tmp/build
ADD files/build/base-deps-func.sh \
    files/build/omnievents.sh \
    ./
RUN bash omnievents.sh && rm *

# IP address for omni services and an auto-configure script and omniORB.cfg
ADD files/etc/omniORB.cfg /etc/omniORB.cfg
ADD files/etc/omnicfg-updater.sh /root/omnicfg-updater.sh
RUN chmod u+x /root/omnicfg-updater.sh && \
    echo "/root/omnicfg-updater.sh" | tee -a /root/.profile

WORKDIR /root

CMD ["/bin/bash", "-l"]

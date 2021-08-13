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

FROM centos:7

ENV RH_VERSION=2.2.8
ENV OMNISERVICEIP=127.0.0.1
ENV OMNISERVICEPORTS=""

LABEL name="REDHAWK SDR Base Image" \
    license="GPLv3" \
    description="REDHAWK SDR repository, omni services, and EPEL)" \
    maintainer="Thomas Goodwin <btgoodwin@geontech.com>" \
    version="${RH_VERSION}" \
    vendor="Geon Technologies, LLC"

# Add redhawk yum repo reference
ADD files/geon-redhawk.repo /etc/yum.repos.d/geon-redhawk.repo

# Update, epel, etc. as well as upgraded pyparsing
# Then add setup supervisord directories
RUN sed -i "s/RH_VERSION/${RH_VERSION}/g" /etc/yum.repos.d/geon-redhawk.repo && \
    yum install -y \
    curl \
    wget \
    epel-release \
    http://cbs.centos.org/kojifiles/packages/pyparsing/2.0.3/1.el7/noarch/pyparsing-2.0.3-1.el7.noarch.rpm && \
    yum install -y python-pip && yum -y clean all && \
    pip install --upgrade "pip < 21.0" && \
    pip install --upgrade supervisor &&  mkdir -p \
        /etc/supervisor.d    \
        /etc/supervisor      \
        /var/log/supervisord

# Supervisord default config
ADD files/supervisord.conf /etc/supervisor/supervisord.conf

# Install omni
RUN yum install -y \
    omniORB-servers \
    omniORB-utils \
    omniEvents-server && \
    yum -y clean all && \
    \
    echo "InitRef = EventService=corbaloc::127.0.0.1:11169/omniEvents" >> /etc/omniORB.cfg

# IP address for omni services and an auto-configure script
ADD files/omnicfg-updater.sh /root/omnicfg-updater.sh
RUN chmod u+x /root/omnicfg-updater.sh && \
    echo "/root/omnicfg-updater.sh" | tee -a /root/.bash_profile

# Polling script for omniEvents dependency on omniNames
ADD files/wait-for-omninames /usr/local/bin/wait-for-omninames
RUN chmod u+x /usr/local/bin/wait-for-*

WORKDIR /root

CMD ["/bin/bash", "-l"]

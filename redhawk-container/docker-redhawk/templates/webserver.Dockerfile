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
LABEL name="REST-Python Web Server" \
    description="Geon's Fork of REST-Python" \
    maintainer="Thomas Goodwin <btgoodwin@geontech.com>"

# Build-time configurable variables
ARG REST_PYTHON=https://github.com/GeonTech/rest-python.git
ARG REST_PYTHON_BRANCH=master
ARG REST_PYTHON_PORT=8080

# Runtime variables
ENV REST_PYTHON=${REST_PYTHON}
ENV REST_PYTHON_BRANCH=${REST_PYTHON_BRANCH}
ENV REST_PYTHON_PORT=${REST_PYTHON_PORT}

# Expose the configured default port.
EXPOSE ${REST_PYTHON_PORT}

RUN yum install -y \
        git \
        gcc \
        python-dev \
        curl \
        python-virtualenv \
        pip && \
    yum clean all -y

# Update pip
RUN  pip install -U "pip < 21.0"

WORKDIR /opt

# Install the rest-python server
RUN git clone -b ${REST_PYTHON_BRANCH} ${REST_PYTHON} && \
    cd rest-python && \
    ./setup.sh install && \
    pip install -r requirements.txt

# Mount point for end-user apps
VOLUME /opt/rest-python/apps

WORKDIR /opt/rest-python

# Supervisord script and "exit" event listener
ADD files/supervisord-rest-python.conf /etc/supervisor.d/rest-python.conf
ADD files/kill_supervisor.py /usr/bin/kill_supervisor.py
RUN chmod u+x /usr/bin/kill_supervisor.py

CMD ["/usr/bin/supervisord"]

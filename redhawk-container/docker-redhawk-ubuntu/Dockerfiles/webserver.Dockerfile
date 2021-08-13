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
LABEL name="REST-Python Web Server" \
    description="Geon's Fork of REST-Python"

# Build-time configurable variables
ARG REST_PYTHON=http://github.com/GeonTech/rest-python.git
ARG REST_PYTHON_BRANCH=master
ARG REST_PYTHON_PORT=8080

# Runtime variables
ENV REST_PYTHON=${REST_PYTHON}
ENV REST_PYTHON_BRANCH=${REST_PYTHON_BRANCH}
ENV REST_PYTHON_PORT=${REST_PYTHON_PORT}

# Expose the configured default port.
EXPOSE ${REST_PYTHON_PORT}

# Install the rest-python server
WORKDIR /opt
ADD files/build/rest-python.sh ./
RUN bash rest-python.sh && rm /opt/rest-python.sh

# Mount point for end-user apps
VOLUME /opt/rest-python/apps

# Move to rest-python
WORKDIR /opt/rest-python

# Supervisord script and "exit" event listener
ADD files/supervisor/supervisord-rest-python.conf /etc/supervisor.d/rest-python.conf
ADD files/supervisor/kill_supervisor.py /usr/bin/kill_supervisor.py
RUN chmod u+x /usr/bin/kill_supervisor.py

CMD ["supervisord"]

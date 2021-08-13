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
LABEL name="REDHAWK IDE Environment" \
    description="REDHAWK Integrated Development Environment Runner"

ENV RHUSER_ID 54321

# Install runtime deps
RUN apt-get update && \
    apt-get install -qy --no-install-recommends \
    	dbus \
        packagekit-gtk3-module \
        libcanberra-gtk-module && \
    rm -rf /var/lib/apt/lists/*

# Add dependencies scripts and builder script.
# Run the builder.
WORKDIR /tmp/build
ADD files/build/base-deps-func.sh \
    files/build/redhawk-source-repo-func.sh \
    files/build/build-sh-process-func.sh \
    files/build/standard-sdrroot.sh \
    files/build/ide-fetcher.py \
    files/build/ide.sh \
    ./
RUN bash standard-sdrroot.sh && \
    bash ide.sh && \
    rm *

# Init script
ADD files/init/rhide-init.sh /opt/rhide-init.sh
RUN chmod a+x /opt/rhide-init.sh

# Exposed volumes
VOLUME /var/redhawk/sdr
VOLUME /home/user/workspace

WORKDIR /root

# Run the RHIDE init script
CMD ["/bin/bash", "-l", "-c", "/opt/rhide-init.sh"]

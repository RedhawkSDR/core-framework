#!/bin/bash
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
set -e

# Set JAVA_HOME and run /etc/profile
export JAVA_HOME=$(readlink -f "/usr/lib/jvm/default-java")
. /etc/profile

# Source some helper functions
source ./base-deps-func.sh
source ./build-sh-process-func.sh

# Install build dependencies.
BUILD_DEPS="${BUILD_DEPS} unzip libusb-1.0.0-dev"
install_build_deps

# Install libnmea
wget http://downloads.sourceforge.net/project/nmea/NmeaLib/nmea-0.5.x/nmealib-0.5.3.zip && \
	unzip nmealib-0.5.3.zip && \
	pushd nmealib && \
	make all-before lib/libnmea.a && cp -r lib include /usr/local && \
	popd && rm -rf nmealib nmealib-0.5.3.zip && \
	ldconfig

# Compile BU353S$
TARGET="BU353S4"
git clone -b 1.0.0 git://github.com/GeonTech/BU353S4.git $TARGET && \
	chmod +x ${TARGET}/nodeconfig.py && \
	build_sh_process ${TARGET} && \
	rm -rf ${TARGET}

# Remove build dependencies
remove_build_deps

# Re-own SDRROOT
chown -R redhawk:redhawk $SDRROOT
chmod -R g+ws $SDRROOT

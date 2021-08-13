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
source ./redhawk-source-repo-func.sh
source ./build-sh-process-func.sh

# Install build dependencies.
BUILD_DEPS="${BUILD_DEPS} libusb-1.0-0-dev"
install_build_deps

# Download the repository
install_repo

# Compile librtlsdr
pushd redhawk-dependencies/librtlsdr
./reconf && ./configure && make -j && make install
popd

# ...and RTL2832U
build_sh_process redhawk-devices/RTL2832U

# Remove the build area up
remove_repo

# Remove build dependencies
remove_build_deps

# Re-own SDRROOT
chown -R redhawk:redhawk $SDRROOT
chmod -R g+ws $SDRROOT

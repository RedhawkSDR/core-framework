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

function std_process() {
	ldconfig
	pushd $1
	./reconf && ./configure CXXFLAGS="-fpermissive"
	make && make install
	popd
}

export JAVA_HOME=$(readlink -f "/usr/lib/jvm/default-java")

source ./base-deps-func.sh
source ./redhawk-source-repo-func.sh

# Install build dependencies.
install_build_deps

# Download the repository
install_repo

# Compile the core-framework (except GPP)
pushd redhawk-core-framework

# redhawk and install the /etc helpers, refresh the environment
std_process redhawk/src
cp -r /usr/local/redhawk/core/etc/* /etc
. /etc/profile

# bulkioInterfaces
std_process bulkioInterfaces

# burstioInterfaces
std_process burstioInterfaces

# frontendInterfaces
std_process frontendInterfaces

# redhawk-codegen
pushd redhawk-codegen
python setup.py install --home=${OSSIEHOME}
popd

# Leave core-framework
popd

# Remove the build area up
remove_repo

# Remove build dependencies
remove_build_deps

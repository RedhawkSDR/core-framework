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

# Install build dependencies.
install_build_deps

# Download the IDE
INSTALL_DIR="${OSSIEHOME}/../ide/${RH_VERSION}"
mkdir -p ${INSTALL_DIR} && pushd ${INSTALL_DIR}
IDE_ASSET="$(python /tmp/build/ide-fetcher.py ${RH_VERSION})"
if ! [ $? -eq  0 ]; then
    echo "Failed to download IDE" 1>&2
    exit 1
fi

# Unpack the asset
echo "Unpacking: ${IDE_ASSET}"
ls -la ${IDE_ASSET}
tar xvzf ${IDE_ASSET}
rm -rf ${IDE_ASSET}
ln -s $PWD/eclipse/eclipse /usr/bin/rhide
popd

# DO NOT remove the build dependencies...

# Re-own SDRROOT
chown -R redhawk:redhawk $SDRROOT
chmod -R g+ws $SDRROOT

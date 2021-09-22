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

RP_BUILD_DEPS="\
	python-dev \
	git \
"

RP_RUN_DEPS="\
	python \
"

function install_deps() {
	apt-get update && apt-get install \
        -qy \
        --no-install-recommends \
        ${RP_BUILD_DEPS} \
        ${RP_RUN_DEPS}
    pip install virtualenv
}

function remove_build_deps() {
    apt-get purge -qy ${RP_BUILD_DEPS}
    apt-get autoremove -qy
    rm -rf /var/lib/apt/lists/*
}

# Install dependencies
install_deps

# Get rest-python, run the setup script
TARGET="rest-python"
git clone -b ${REST_PYTHON_BRANCH} ${REST_PYTHON} ${TARGET}
pushd ${TARGET}
./setup.sh install && pip install -r requirements.txt
popd

# Remove dependencies
remove_build_deps

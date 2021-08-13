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

# Referenced from https://files.ettus.com/manual/page_build_guide.html
UHD_BUILD_DEPS="\
    git \
    libboost-all-dev \
    libusb-1.0-0-dev \
    python-mako \
    doxygen \
    python-docutils \
    cmake \
    build-essential \
    "

UHD_RUN_DEPS="\
    libboost-chrono1.58.0 \
    libboost-date-time1.58.0 \
    libboost-filesystem1.58.0 \
    libboost-program-options1.58.0 \
    libboost-regex1.58.0 \
    libboost-system1.58.0 \
    libboost-serialization1.58.0 \
    libusb-1.0-0 \
    "

function install_uhd_deps() {
    apt-get update && apt-get install \
        -qy \
        --no-install-recommends \
        ${UHD_BUILD_DEPS} \
        ${UHD_RUN_DEPS}
    pip install --upgrade requests
}

function remove_uhd_build_deps() {
    apt-get purge -qy ${UHD_BUILD_DEPS}
    apt-get autoremove -qy
    # rm -rf /var/lib/apt/lists/*
}

function install_uhd() {
    # Install all dependencies
    install_uhd_deps

    # Clone, compile, install
    git clone -b release_003_010_001_001 git://github.com/EttusResearch/uhd.git uhd && \
        mkdir -p uhd/host/build && \
        pushd uhd/host/build && \
        cmake .. && \
        make && \
        make test && \
        make install && \
        ldconfig && \
        cpack ../
    popd
    rm -rf uhd

    # Remove build dependencies
    remove_uhd_build_deps
}



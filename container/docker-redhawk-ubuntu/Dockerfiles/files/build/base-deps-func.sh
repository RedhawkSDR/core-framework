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
BUILD_DEPS="\
    build-essential \
    git \
    wget \
    autoconf \
    automake \
    libtool \
    libexpat1-dev \
    uuid-dev \
    libfftw3-dev \
    liblog4cxx-dev \
    python-dev \
    pyqt4-dev-tools \
    libboost-date-time-dev \
    libboost-filesystem-dev \
    libboost-regex-dev \
    libboost-serialization-dev \
    libboost-system-dev \
    libboost-thread-dev \
    libboost-iostreams-dev \
    libxerces-c-dev \
    libapr1-dev \
    libaprutil1-dev \
    libomniorb4-dev \
    libcos4-dev \
    xsdcxx"

function install_build_deps() {
    apt-get update && apt-get install -qy --no-install-recommends ${BUILD_DEPS}
    # Patch XSD 4.0.0
    cat<<EOF | tee ./xsd-4.0.0-expat.patch
diff --git a/libxsd/xsd/cxx/parser/expat/elements.txx b/libxsd/xsd/cxx/parser/expat/elements.txx
index ef9adb7..8df4d67 100644
--- a/libxsd/xsd/cxx/parser/expat/elements.txx
+++ b/libxsd/xsd/cxx/parser/expat/elements.txx
@@ -279,7 +279,7 @@ namespace xsd
         {
           parser_auto_ptr parser (XML_ParserCreateNS (0, XML_Char (' ')));
 
-          if (parser == 0)
+          if (parser.get () == 0)
             throw std::bad_alloc ();
 
           if (system_id || public_id)
EOF
    patch /usr/include/xsd/cxx/parser/expat/elements.txx xsd-4.0.0-expat.patch
}

function remove_build_deps() {
    apt-get purge -qy ${BUILD_DEPS}
    apt-get autoremove -qy
    rm -rf /var/lib/apt/lists/*
}

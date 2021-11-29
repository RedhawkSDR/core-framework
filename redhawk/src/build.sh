#!/usr/bin/env bash
#
# This file is protected by Copyright. Please refer to the COPYRIGHT file 
# distributed with this source distribution.
# 
# This file is part of REDHAWK core.
# 
# REDHAWK core is free software: you can redistribute it and/or modify it under 
# the terms of the GNU Lesser General Public License as published by the Free 
# Software Foundation, either version 3 of the License, or (at your option) any 
# later version.
# 
# REDHAWK core is distributed in the hope that it will be useful, but WITHOUT 
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
# 
# You should have received a copy of the GNU Lesser General Public License 
# along with this program.  If not, see http://www.gnu.org/licenses/.
#

config_ac='configure.ac'
make_am='Makefile.am'
makefile='Makefile'
uname=$(uname)

set -e

if [ "$1" == "clean" ]; then
 make clean
else
 if [[ $config_ac -nt $makefile || $make_am -nt $makefile ]]; then
  set -x
  ./reconf
  if [[ $uname = "Darwin" ]]; then
    PYTHON=python3.9 CXXFLAGS="-std=c++14 -Wno-deprecated-declarations" ./configure -C -with-expat=/usr/local/Cellar/expat/2.4.1  --disable-log4cxx --without-tests --disable-java --disable-persistence
  else
    CXXFLAGS=-Wno-deprecated RT_LIB=-lrt ./configure -C --without-tests --disable-java --disable-persistence
  fi
 fi
 make -j
fi

#!/bin/sh
#
# This file is protected by Copyright. Please refer to the COPYRIGHT file
# distributed with this source distribution.
#
# This file is part of REDHAWK bulkioInterfaces.
#
# REDHAWK bulkioInterfaces is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
#
# REDHAWK bulkioInterfaces is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see http://www.gnu.org/licenses/.
#
configure='configure'
makefile_in='Makefile.in'
config_ac='configure.ac'
make_am='Makefile.am'
makefile='Makefile'

if [ "$1" == 'clean' ]; then
  make clean
else
  # Checks if build is newer than makefile (based on modification time)
  if [[ ! -e $configure || ! -e $makefile_in || $config_ac -nt $makefile || $make_am -nt $makefile ]]; then
    ./reconf
    ./configure
  fi
  make
  exit 0
fi

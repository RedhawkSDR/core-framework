#!/bin/bash
# This file is protected by Copyright. Please refer to the COPYRIGHT file
# distributed with this source distribution.
#
# This file is part of Docker REDHAWK.
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

function print_option() {
	printf " ---> %-20s %-20s\n" $1 $2
}

BU353S4_CONFIG_ARGS="--noinplace --domainname=${DOMAINNAME} --nodename=${NODENAME} --serialport=${GPS_PORT}"

if ! [ -d $SDRROOT/dev/nodes/${NODENAME} ]; then
	echo Configuring BU353S4 Node

	${SDRROOT}/dev/devices/BU353S4/nodeconfig.py ${BU353S4_CONFIG_ARGS}
else
	echo BU353S4 Node already configured
fi


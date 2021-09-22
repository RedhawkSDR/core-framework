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

export NODENAME=${NODENAME:-MyRtlsdr_$(hostname -s)}

RTL2832U_CONFIG_ARGS="--noinplace --domainname=${DOMAINNAME} --nodename=${NODENAME}"

if ! [ -d $SDRROOT/dev/nodes/${NODENAME} ]; then
	echo Configuring RTL2832U Node

	if ! [ -z ${RTL_NAME} ]; then
		print_option Name\: ${RTL_NAME}
		RTL2832U_CONFIG_ARGS="${RTL2832U_CONFIG_ARGS} --rtlname=${RTL_NAME}"
	fi

	if ! [ -z ${RTL_VENDOR} ]; then
		print_option Vendor\: ${RTL_VENDOR}
		RTL2832U_CONFIG_ARGS="${RTL2832U_CONFIG_ARGS} --rtlvendor=${RTL_VENDOR}"
	fi

	if ! [ -z ${RTL_PRODUCT} ]; then
		print_option Product\: ${RTL_PRODUCT}
		RTL2832U_CONFIG_ARGS="${RTL2832U_CONFIG_ARGS} --rtlproduct=${RTL_PRODUCT}"
	fi

	if ! [ -z ${RTL_SERIAL} ]; then
		print_option Serial\: ${RTL_SERIAL}
		RTL2832U_CONFIG_ARGS="${RTL2832U_CONFIG_ARGS} --rtlserial=${RTL_SERIAL}"
	fi

	if ! [ -z ${RTL_INDEX} ]; then
		print_option Index\: ${RTL_INDEX}
		RTL2832U_CONFIG_ARGS="${RTL2832U_CONFIG_ARGS} --rtlindex=${RTL_INDEX}"
	fi

	# Patch the nodeconfig.py script so we can pass simple parameters on the command line with $NODEFLAGS
	patch ${SDRROOT}/dev/devices/rh/RTL2832U/nodeconfig.py < /root/rtl2832u-nodeconfig.patch

	# Configure the node
	${SDRROOT}/dev/devices/rh/RTL2832U/nodeconfig.py ${RTL2832U_CONFIG_ARGS} ${NODEFLAGS}
else
	echo RTL2832U Node already configured
fi

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

export NODENAME=${NODENAME:-MyUsrp_$(hostname -s)}

USRP_CONFIG_ARGS="--noinplace --domainname=${DOMAINNAME} --nodename=${NODENAME} --usrpname=${USRP_NAME}"

if ! [ -d $SDRROOT/dev/nodes/${NODENAME} ]; then
	echo Configuring USRP Node
	if ! [ -z ${USRP_IP_ADDRESS} ]; then
		print_option IP\: ${USRP_IP_ADDRESS}
		USRP_CONFIG_ARGS="${USRP_CONFIG_ARGS} --usrpip=${USRP_IP_ADDRESS}"
	fi

	if ! [ -z ${USRP_TYPE} ]; then
		print_option Type\: ${USRP_TYPE}
		USRP_CONFIG_ARGS="${USRP_CONFIG_ARGS} --usrptype=${USRP_TYPE}"
	fi

	if ! [ -z ${USRP_SERIAL} ]; then
		print_option Serial\: ${USRP_SERIAL}
		USRP_CONFIG_ARGS="${USRP_CONFIG_ARGS} --usrpserial=${USRP_SERIAL}"
	fi

	# Patch the nodeconfig.py script so we can pass simple parameters on the command line with $NODEFLAGS
	patch ${SDRROOT}/dev/devices/rh/USRP_UHD/nodeconfig.py < /root/usrp-nodeconfig.patch

	# Configure the node
	${SDRROOT}/dev/devices/rh/USRP_UHD/nodeconfig.py ${USRP_CONFIG_ARGS} ${NODEFLAGS}
else
	echo USRP Node already configured
fi
